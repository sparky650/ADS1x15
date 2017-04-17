/**
 * @file	wireUtil.h
 * @author	Keegan Morrow
 * @version	1.1.2
 * @brief Utility base class for reading and writing registers on i2c devices
 *
 */

#ifndef __wireUtil_h_
#define __wireUtil_h_

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Utility base class for reading and writing registers on i2c devices
 *
 * @tparam REGTYPE An initialized enum type that lists the valid registers for the device
 * @tparam DATATYPE = uint8_t Data type (register size) supports uint8_t, uint16_t, uint32_t
 */
template <typename REGTYPE, typename DATATYPE = uint8_t>
class wireUtil
{
public:
	/**
	 * @brief Attach a function to be called on a read timeout
	 *
	 * @param timeOutHandler Pointer to a 'void f(void)' function
	 */
	void attachTimeoutHandler(void (*timeOutHandler)(void)) {this->timeOutHandler = timeOutHandler;}
	/**
	 * @brief Attach a function to be called on a write NACK
	 *
	 * @param errorHandler Pointer to a 'void f(uint8_t)' function. This will be passed the Wire status.
	 */
	void attachErrorHandler(void (*errorHandler)(uint8_t)) {this->errorHandler = errorHandler;}

	unsigned long timeoutTime; ///< Amount of time to wait for a successful read
	bool timeoutFlag; ///< Set to true if there is a timeout event, reset on the next read

	/**
	 * @brief Safe method to read the state of the timeout flag
	 *
	 * @return State of the timeout flag
	 */
	inline bool getTimeoutFlag() {return timeoutFlag;}

	virtual void begin();
	virtual void begin(uint8_t);
#if defined(ARDUINO_ARCH_ESP8266)
	virtual void begin(uint8_t, uint8_t, uint8_t);
#endif // ARDUINO_ARCH_ESP8266

	bool writeRegister(REGTYPE, DATATYPE);
	bool writeRegisters(REGTYPE, DATATYPE *, uint8_t);
	DATATYPE readRegister(REGTYPE);
	bool readRegisters(REGTYPE, DATATYPE *, uint8_t);
	bool setRegisterBit(REGTYPE, uint8_t, bool);

protected:
	uint8_t address; ///< Hardware address of the device

private:
	void (*timeOutHandler)(void);
	void (*errorHandler)(uint8_t);
	void writeAsBytes(DATATYPE);
	DATATYPE readAsBytes();
};

/*
* Note: Member functions that use template must be
* defined in this file.
*/

/**
 * @brief Initialize the chip at a specific address
 *
 * @param address  Address of the chip
 */
template <typename REGTYPE, typename DATATYPE>
void wireUtil<REGTYPE, DATATYPE>::begin(uint8_t address)
{
	this->address = address;
	Wire.begin();
}

#if defined(ARDUINO_ARCH_ESP8266)
/**
 * @brief Initialize the chip at a specific address and pins
 * @details This is only available on architectures that support arbitrary SDA and SCL pins.
 *
 * @param address Address of the chip
 * @param SDApin Pin number for the SDA signal
 * @param SCLpin Pin number for the SCL signal
 */
template <typename REGTYPE, typename DATATYPE>
void wireUtil<REGTYPE, DATATYPE>::begin(uint8_t address, uint8_t SDApin, uint8_t SCLpin)
{
	this->address = address;
	Wire.begin(SDApin, SCLpin);
}
#endif // ARDUINO_ARCH_ESP8266

/**
 * @brief Write a single register on an i2c device
 *
 * @param reg Register address (from a device specific enum)
 * @param data Data to be written to the device
 * @return true on success, false if NACK
 */
template <typename REGTYPE, typename DATATYPE>
bool wireUtil<REGTYPE, DATATYPE>::writeRegister(REGTYPE reg, DATATYPE data)
{
	return writeRegisters(reg, &data, 1);
}

/**
 * @brief Write to a sequence of registers on an i2c device
 *
 * @param reg First register address (from a device specific enum)
 * @param buffer Array containing the data to be written
 * @param len Number of bytes in the array
 * @return true on success, false if NACK
 */
template <typename REGTYPE, typename DATATYPE>
bool wireUtil<REGTYPE, DATATYPE>::writeRegisters(REGTYPE reg, DATATYPE *buffer, uint8_t len)
{
	Wire.beginTransmission(address);
	Wire.write(reg);
	for (uint8_t i = 0; i < len; i++)
	{
		if (sizeof(DATATYPE) == 1) { Wire.write(buffer[i]); }
		else { writeAsBytes(buffer[i]); }
	}

	uint8_t status;
	status = Wire.endTransmission();
	if (status == 0) { return true; }
	else if (errorHandler != NULL)
	{
		(*errorHandler)(status);
		return false;
	}
	return false;
}

/**
 * @brief Read a single register from an i2c device
 *
 * @param reg Register address (from a device specific enum)
 * @return Data from the device register, 0 if there is a timeout
 */
template <typename REGTYPE, typename DATATYPE>
DATATYPE wireUtil<REGTYPE, DATATYPE>::readRegister(REGTYPE reg)
{
	unsigned long abortTime;
	Wire.beginTransmission(address);
	Wire.write((uint8_t)reg);
	Wire.endTransmission(false);
	Wire.requestFrom(address, (uint8_t) sizeof(DATATYPE));

	timeoutFlag = false;
	abortTime = millis() + timeoutTime;
	while (!Wire.available())
	{
		if (abortTime < millis())
		{
			timeoutFlag = true;
			if (timeOutHandler != NULL) { (*timeOutHandler)(); }
			return 0;
		}
	}
	if (sizeof(DATATYPE) == 1) { return Wire.read(); }
	else { return readAsBytes(); }
}

/**
 * @brief Read a number of sequential registers from an i2c device
 *
 * @param reg First register address (from a device specific enum)
 * @param buffer Array to contain the data read
 * @param len Number of bytes to read
 * @return true on success, false on timeout
 */
template <typename REGTYPE, typename DATATYPE>
bool wireUtil<REGTYPE, DATATYPE>::readRegisters(REGTYPE reg, DATATYPE *buffer, uint8_t len)
{
	Wire.beginTransmission(address);
	Wire.write((uint8_t)reg);
	Wire.endTransmission(false);

	timeoutFlag = false;
	unsigned long abortTime = millis() + timeoutTime;
	size_t bufferSize = len * sizeof(DATATYPE);
	memset(buffer, 0, bufferSize);

	Wire.requestFrom(address, (uint8_t) bufferSize);

	while (Wire.available() < bufferSize)
	{
		if (abortTime < millis())
		{
			timeoutFlag = true;
			if (timeOutHandler != NULL) { (*timeOutHandler)(); }
			return false; // timeout condition, return false
		}
	}

	for (uint8_t i = 0 ; i < len ; i++)
	{
		if (sizeof(DATATYPE) == 1) { buffer[i] = Wire.read(); }
		else { buffer[i] = readAsBytes(); }
	}

	return true;
}

/**
 * @brief Read modify write a bit on a register
 *
 * @param reg register to modify
 * @param bit index of the bit to set
 * @param state state of the bit to set
 * @return true on success
 */
template <typename REGTYPE, typename DATATYPE>
bool wireUtil<REGTYPE, DATATYPE>::setRegisterBit(REGTYPE reg, uint8_t bit, bool state)
{
	DATATYPE tempReg;
	tempReg = readRegister(reg);
	if (state) { tempReg |= (DATATYPE)(1 << bit); }
	else { tempReg &= ~(DATATYPE)(1 << bit); }
	return writeRegister(reg, tempReg);
}

/**
 * @brief Assembles and writes a big endian packet
 *
 * @param d Unit of data to be written
 */
template <typename REGTYPE, typename DATATYPE>
void wireUtil<REGTYPE, DATATYPE>::writeAsBytes(DATATYPE d)
{
	uint8_t buffer[sizeof(DATATYPE)];
	switch (sizeof(DATATYPE))
	{
	case 4:
		buffer[3] = d & 0xFF;
		d = d >> 8;
		buffer[2] = d & 0xFF;
		d = d >> 8;
		buffer[1] = d & 0xFF;
		d = d >> 8;
		buffer[0] = d & 0xFF;
		break;
	case 2:
		buffer[1] = d & 0xFF;
		buffer[0] = (d >> 8) & 0xFF;
		break;
	case 1:
		buffer[0] = d;
		break;
	default:
		memset(buffer, 0x00, sizeof(DATATYPE));
	}
	Wire.write(buffer, sizeof(DATATYPE));
}

/**
 * @brief Receives a big endian packet and converts to a data unit
 *
 * @return The re-assembled data unit
 */
template <typename REGTYPE, typename DATATYPE>
DATATYPE wireUtil<REGTYPE, DATATYPE>::readAsBytes()
{
	DATATYPE d;
	switch (sizeof(DATATYPE))
	{
	case 4:
		d = (uint32_t)Wire.read() << 24;
		d |= (uint32_t)Wire.read() << 16;
		d |= (uint32_t)Wire.read() << 8;
		d |= (uint32_t)Wire.read();
		break;
	case 2:
		d = (uint16_t)Wire.read() << 8;
		d |= (uint16_t)Wire.read();
		break;
	case 1:
		d = (uint8_t)Wire.read();
		break;
	default:
		d = 0;
	}
	return d;
}

#endif // __wireUtil_h_