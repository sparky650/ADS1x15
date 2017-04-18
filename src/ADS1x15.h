/**
 * @file ADS1115.h
 * @author Keegan Morrow
 * @version 0.0.2
 * @brief Classes for the ADS1015 and ADS1115 analog to digital converters
 */

#ifndef __ADS1x15_h_
#define __ADS1x15_h_

#include <Arduino.h>
#include <Wire.h>
#include "utility/wireUtil.h"

enum ADS1x15_Register_t
{
	CONVERSION_REG = 0x00,
	CONFIG_REG = 0x01,
	LOW_THRESH_REG = 0x02,
	HI_THRESH_REG = 0x03
};

static const uint16_t ADS1x15_OS = 1 << 15;

static const uint16_t ADS1x15_MUX_MASK = (0x07) << 12;
enum ADS1x15_MUX_t
{
	DIF01 = (0x0 << 12),
	DIF03 = (0x1 << 12),
	DIF13 = (0x2 << 12),
	DIF23 = (0x3 << 12),
	SE0 = (0x4 << 12),
	SE1 = (0x5 << 12),
	SE2 = (0x6 << 12),
	SE3 = (0x7 << 12)
};

static const uint16_t ADS1x15_GAIN_MASK = (0x7) << 9;
enum ADS1x15_GAIN_t
{
	GAIN_23 = (0x0 << 9),
	GAIN_1 = (0x1 << 9),
	GAIN_2 = (0x2 << 9),
	GAIN_4 = (0x3 << 9),
	GAIN_8 = (0x4 << 9),
	GAIN_16 = (0x5 << 9)
};

static const uint16_t ADS1x15_MODE_MASK = 1 << 8;
enum ADS1x15_MODE_t
{
	CONTINUOUS_CONV = 0x0 << 8,
	SINGLE_SHOT = 0x1 << 8
};

static const uint16_t ADS1x15_DR_MASK = (0x7) << 5;
enum ADS1115_DR_t
{
	ADS1115_DR_8 = (0x0 << 5),
	ADS1115_DR_16 = (0x1 << 5),
	ADS1115_DR_32 = (0x2 << 5),
	ADS1115_DR_64 = (0x3 << 5),
	ADS1115_DR_128 = (0x4 << 5),
	ADS1115_DR_250 = (0x5 << 5),
	ADS1115_DR_475 = (0x6 << 5),
	ADS1115_DR_860 = (0x7 << 5)
};
enum ADS1015_DR_t
{
	ADS1015_DR_128 = (0x0 << 5),
	ADS1015_DR_250 = (0x1 << 5),
	ADS1015_DR_490 = (0x2 << 5),
	ADS1015_DR_920 = (0x3 << 5),
	ADS1015_DR_1600 = (0x4 << 5),
	ADS1015_DR_2400 = (0x5 << 5),
	ADS1015_DR_3300 = (0x6 << 5)
};

static const uint16_t ADS1x15_COMP_MODE_MASK = 1 << 4;
enum ADS1x15_COMP_MODE_t
{
	STANDARD_COMP = 0x0 << 4,
	WINDOW_COMP = 0x1 << 4
};

static const uint16_t ADS1x15_COMP_POL_MASK = 1 << 3;
enum ADS1x15_COMP_POL_t
{
	ACTIVE_LOW = 0x0 << 3,
	ACTIVE_HIGH = 0x1 << 3
};

static const uint16_t ADS1x15_COMP_LAT_MASK = 1 << 2;
enum ADS1x15_COMP_LAT_t
{
	NONLATCHING_COMP = 0x0 << 2,
	LATCHING_COMP = 0x1 << 2
};

static const uint16_t ADS1x15_QUE_MASK = 0x3;
enum ADS1x15_QUE_t
{
	QUE_ONE = 0x0,
	QUE_TWO = 0x1,
	QUE_FOUR = 0x2,
	QUE_DISABLE = 0x3
};

typedef ADS1x15_GAIN_t ADS1015_GAIN_t;
typedef ADS1x15_GAIN_t ADS1115_GAIN_t;

static const uint16_t defaultConfig = 0x8583; // default startup state (from the datasheet)

static const uint8_t defaultAddress = 0x48;

/**
 * @brief Foundation class for the ADS1015 and ADS1115 ADCs
 */
class ADS1x15: public wireUtil<ADS1x15_Register_t, uint16_t>
{
public:
	using wireUtil::begin;
	ADS1x15()
	{
		timeoutTime = 1000UL;
		timeoutFlag = false;
		calibration = 1.0;
		configRegister = defaultConfig;
		currentGain = GAIN_2; // this needs to match the defaultConfig configuration
	}
	void begin() {begin(defaultAddress);}
	inline uint8_t addressIndex(uint8_t a) {return a + defaultAddress;}
	void setCalibration(float);
	void setCalibration(float, float);
	void setGain(ADS1x15_GAIN_t);
	float getFullScaleV();
	void setComparatorMode(ADS1x15_COMP_MODE_t);
	void setComparatorPolarity(ADS1x15_COMP_POL_t);
	void setComparatorLatch(ADS1x15_COMP_LAT_t);
	int16_t analogRead(ADS1x15_MUX_t);
	uint16_t analogRead(uint8_t);
	float analogReadVoltage(uint8_t);
	float analogReadCurrent(uint8_t, float = 100.0);
	float analogRead420(uint8_t, float = 100.0);
	inline float getCalibration() {return calibration;}
	virtual uint8_t getADCbits();
	virtual uint16_t getFullScaleBits();

protected:
	uint16_t configRegister;
	ADS1x15_GAIN_t currentGain;
	uint32_t conversionDelay;
	float calibration;
	virtual inline uint16_t shiftConversion(uint16_t c) {return c;}
};

/**
 * @brief Interface class for the ADS1115 analog to digital converter
 */
class ADS1115: public ADS1x15
{
public:
	ADS1115()
	{
		ADS1x15();
		setDataRate(ADS1115_DR_128);
	}
	void setDataRate(ADS1115_DR_t);
	/**
	 * @brief Get the number of bits of the current ADC
	 *
	 * @return Number of bits
	 */
	inline uint8_t getADCbits() {return 16;}
	/**
	 * @brief Get the full scale binary output for the chip
	 *
	 * @return Full scale output
	 */
	inline uint16_t getFullScaleBits() {return 0x7FFF;}
};

/**
 * @brief Interface class for the ADS1015 analog to digital converter
 */
class ADS1015: public ADS1x15
{
public:
	ADS1015()
	{
		ADS1x15();
		setDataRate(ADS1015_DR_1600);
	}
	void setDataRate(ADS1015_DR_t);
	/**
	 * @brief Get the number of bits of the current ADC
	 *
	 * @return Number of bits
	 */
	inline uint8_t getADCbits() {return 12;}
	/**
	 * @brief Get the full scale binary output for the chip
	 *
	 * @return Full scale output
	 */
	inline uint16_t getFullScaleBits() {return 0x07FF;}
private:
	inline uint16_t shiftConversion(uint16_t c) {return c >> 4;}
};


#endif // __ADS1115_h_