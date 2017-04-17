/**
 * @file ADS1115.h
 * @author Keegan Morrow
 * @version 0.0.1
 *
 */

#ifndef __ADS1x15_h_
#define __ADS1x15_h_

#include <Arduino.h>
#include <Wire.h>
#include "utility/wireUtil.h"

enum ADS1x15_Register_t
{
	CONVERSION = 0x00,
	CONFIG = 0x01,
	LOW_THRESH = 0x02,
	HI_THRESH = 0x03
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

typedef ADS1x15_GAIN_t ADS1015_GAIN_t;
typedef ADS1x15_GAIN_t ADS1115_GAIN_t;

static const uint8_t defaultAddress = 0x48;

static const float fullScaleV[] =
{ 6.144, 4.096, 2.048, 1.024, 0.512, 0.256 };



class ADS1x15: public wireUtil<ADS1x15_Register_t, uint16_t>
{
public:
	using wireUtil::begin;
	ADS1x15()
	{
		timeoutTime = 1000UL;
		timeoutFlag = false;
	}
	void begin() {begin(defaultAddress);}

	uint8_t addressIndex(uint8_t a) {return a + defaultAddress;}

	void setGain(ADS1x15_GAIN_t);

	//float getFullScaleV();

	uint16_t analogRead(uint8_t);
	uint16_t analogReadVoltage(uint8_t);
	uint16_t analogReadCurrent(uint8_t);

private:
	uint16_t configRegister;
};

class ADS1115: public ADS1x15
{
public:
	ADS1115() {ADS1x15();}
	void setDataRate(ADS1115_DR_t);
};

class ADS1015: public ADS1x15
{
public:
	ADS1015() {ADS1x15();}
	void setDataRate(ADS1015_DR_t);
};


#endif // __ADS1115_h_