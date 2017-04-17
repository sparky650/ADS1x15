#include "ADS1x15.h"

/**
 * @brief Set the calibration factor for calculating the voltage or current input
 *
 * @param calibration Correction factor
 */
void ADS1x15::setCalibration(float calibration)
{
	this->calibration = calibration;
}

/**
 * @brief Calculate the calibration factor for calculating the voltage or current input
 *
 * @param r1 First resistor in the resistor divider
 * @param r2 Second resistor in the resistor divider
 */
void ADS1x15::setCalibration(float r1, float r2)
{
	if (r2 > 0.0) { calibration = (r1 + r2) / r2; }
}

/**
 * @brief Set the gain value for the programmable gain amplifier
 *
 * @param currentGain Gain value from ADS1x15_GAIN_t
 */
void ADS1x15::setGain(ADS1x15_GAIN_t currentGain)
{
	this->currentGain = currentGain;
	configRegister &= ~(uint16_t)ADS1x15_GAIN_MASK;
	configRegister |= (uint16_t)currentGain;
}

/**
 * @brief Get the current full scale value in V
 *
 * @return Voltage based on the current gain and calibration factor
 */
float ADS1x15::getFullScaleV()
{
	float value = 0.0;
	if (currentGain == GAIN_23) {value = 6.144;}
	else if (currentGain == GAIN_1) {value = 4.096;}
	else if (currentGain == GAIN_2) {value = 2.048;}
	else if (currentGain == GAIN_4) {value = 1.024;}
	else if (currentGain == GAIN_8) {value = 0.512;}
	else if (currentGain == GAIN_16) {value = 0.256;}
	return value * calibration;
}

/**
 * @brief Set the mode of the comparator.
 *
 * @param compCfg Configuration to set
 */
void ADS1x15::setComparatorMode(ADS1x15_COMP_MODE_t compCfg)
{
	configRegister &= ~(uint16_t)ADS1x15_COMP_MODE_MASK;
	configRegister |= (uint16_t)compCfg;
}

/**
 * @brief Set the polarity of the comparator.
 *
 * @param compCfg Configuration to set
 */
void ADS1x15::setComparatorPolarity(ADS1x15_COMP_POL_t compCfg)
{
	configRegister &= ~(uint16_t)ADS1x15_COMP_POL_MASK;
	configRegister |= (uint16_t)compCfg;
}

/**
 * @brief Set the latching mode of the comparator.
 *
 * @param compCfg Configuration to set
 */
void ADS1x15::setComparatorLatch(ADS1x15_COMP_LAT_t compCfg)
{
	configRegister &= ~(uint16_t)ADS1x15_COMP_LAT_MASK;
	configRegister |= (uint16_t)compCfg;
}

/**
 * @brief Read an analog value
 *
 * @param m The configuration of the MUX
 * @return The converted value
 */
int16_t ADS1x15::analogRead(ADS1x15_MUX_t mux)
{
	uint16_t result;
	configRegister &= ~(uint16_t)ADS1x15_MUX_MASK;
	configRegister |= (uint16_t)mux;
	configRegister |= ADS1x15_OS;
	writeRegister(CONFIG_REG, configRegister);
	delayMicroseconds(conversionDelay);
	result = shiftConversion(readRegister(CONVERSION_REG));
	if (result > getFullScaleBits()) { return (int16_t)(result | 0x8000); }
	else { return (int16_t)result; }
}

/**
 * @brief Read an analog value
 *
 * @param ch The input channel to read
 * @return The converted value
 */
uint16_t ADS1x15::analogRead(uint8_t ch)
{
	if (ch == 0) { return analogRead(SE0); }
	else if (ch == 1) { return analogRead(SE1); }
	else if (ch == 2) { return analogRead(SE2); }
	else if (ch == 3) { return analogRead(SE3); }
	return 0;
}

/**
 * @brief Read an input and calculate the voltage based on the current gain settings
 *
 * @param ch The input channel to read
 * @return The converted value in V
 */
float ADS1x15::analogReadVoltage(uint8_t ch)
{
	return getFullScaleV() * ((float)analogRead(ch) / (float)getFullScaleBits());
	// return ((float)analogRead(ch) / (float)getFullScaleBits());
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param ch The input channel to read
 * @param r Burden resistor value in ohms
 *
 * @return The converted value in mA
 */
float ADS1x15::analogReadCurrent(uint8_t ch, float r)
{
	if (r > 0.0) { return analogReadVoltage(ch) / r; }
	else { return 0.0; }
}

/**
 * @brief Read the output from a 4-20mA device in %
 *
 * @param ch The input channel to read
 * @param r Burden resistor value in ohms
 *
 * @return The converted value in %
 */
float ADS1x15::analogRead420(uint8_t ch, float r)
{
	return (analogReadCurrent(ch, r) - 4.0) / 16.0;
}

/**
 * @brief Set the conversion rate in samples per second
 *
 * @param dataRate One of the rate settings from ADS1115_DR_t
 */
void ADS1115::setDataRate(ADS1115_DR_t dataRate)
{
	configRegister &= ~(uint16_t)ADS1x15_DR_MASK;
	configRegister |= (uint16_t)dataRate;
	if (dataRate == ADS1115_DR_8) { conversionDelay = 125400; }
	if (dataRate == ADS1115_DR_16) { conversionDelay = 62900; }
	if (dataRate == ADS1115_DR_32) { conversionDelay = 31650; }
	if (dataRate == ADS1115_DR_64) { conversionDelay = 16025; }
	if (dataRate == ADS1115_DR_128) { conversionDelay = 8213; }
	if (dataRate == ADS1115_DR_250) { conversionDelay = 4400; }
	if (dataRate == ADS1115_DR_475) { conversionDelay = 2505; }
	if (dataRate == ADS1115_DR_860) { conversionDelay = 1563; }
}

/**
 * @brief Set the conversion rate in samples per second
 *
 * @param dataRate One of the rate settings from ADS1015_DR_t
 */
void ADS1015::setDataRate(ADS1015_DR_t dataRate)
{
	configRegister &= ~ADS1x15_DR_MASK;
	configRegister |= (uint16_t)dataRate;
	if (dataRate == ADS1015_DR_128) { conversionDelay = 8213; }
	if (dataRate == ADS1015_DR_250) { conversionDelay = 4400; }
	if (dataRate == ADS1015_DR_490) { conversionDelay = 2441; }
	if (dataRate == ADS1015_DR_920) { conversionDelay = 1487; }
	if (dataRate == ADS1015_DR_1600) { conversionDelay = 1025; }
	if (dataRate == ADS1015_DR_2400) { conversionDelay = 817; }
	if (dataRate == ADS1015_DR_3300) { conversionDelay = 703; }
}
