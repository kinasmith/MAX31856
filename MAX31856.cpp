/**
 * @file MAX31856.cpp
 * This is the library to access the MAX31856
 *
 * Copyright (c) 2016 Kina Smith
 * This software is released under the MIT license. See the attached LICENSE file for details.
 */

#include "MAX31856.h"

// SPISettings MAX31856::spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
SPISettings MAX31856::_settings = SPISettings(1000000, MSBFIRST, SPI_MODE1);

MAX31856::MAX31856(uint8_t CSx, uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT) {
  SPI.begin();
  // _settings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
  _cs = CSx;
  _tcType = TC_TYPE;
  _filtFreq = FILT_FREQ;
  _avgMode = AVG_MODE;
  _cMode = CMODE;
  _oneShot = ONE_SHOT;

  this->internal = 0;
  this->external = 0;
  this->fault = 0;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  uint8_t regdat = 0;   // set up paramater to compile register configs
  // set CR0 (REG_CR0)
  // regdat = (CMODE | ONE_SHOT | OCFAULT_10MS | CJ_ENABLED | FAULT_AUTO | FAULT_CLR_DEF | FILT_FREQ);
  regdat = (_cMode | _oneShot | OCFAULT_10MS | CJ_ENABLED | FAULT_AUTO | FAULT_CLR_DEF | _filtFreq);
  regWrite(REG_CR0, 0xFF, regdat); // write data to register
/*  CRO, 00h/80h:
    [7] cmode (0=off (default), 1=auto conv mode)
    [6] 1shot (0=off, default)
    [5:4] OCFAULT (table 4 in datasheet)
    [3] CJ disable (0=cold junction enabled by default, 1=CJ disabled, used to write CJ temp)
    [2] FAULT mode (0=sets, clears automatically, 1=manually cleared, sets automatically)
    [1] FAULTCLR   (0 - default, 1=see datasheet)
    [0] 50/60Hz (0=60hz (default), 1=50Hz filtering) + harmonics */

  // set CR1 (REG_CR1)
  // regdat = (AVG_MODE | TC_TYPE);
  regdat = (_avgMode | _tcType);
  regWrite(REG_CR1, 0xFF, regdat);
/*  CR1, 01h/81h:
    [7] reserved
    [6:4] AVGSEL (0=1samp(default),1=2samp,2=4samp,3=8samp,0b1xx=16samp])
    [3:0] TC type (0=B, 1=E, 2=J, 3=K(default), 4=N, 5=R, 6=S, 7=T, others, see datasheet)*/

  // set MASK (REG_MASK) - PWF default masks all but OV/UV and OPEN from lighting LED
  regdat = (CJ_HIGH_MASK | CJ_LOW_MASK | TC_HIGH_MASK | TC_LOW_MASK);
  regWrite(REG_MASK, 0x3F, regdat);
/*  MASK, 02h/82h: This register masks faults from causing the FAULT output from asserting,
           but fault bits will still be set in the FSR (0x0F)
               All faults are masked by default... must turn them on if desired
    [7:6] reserved
    [5] CJ high fault mask
    [4] CJ low fault mask
    [3] TC high fault mask
    [2] TC low fault mask
    [1] OV/UV fault mask
    [0] Open fault mask
    PWF example: 0x03 (OV/UV + open) */

  // LEAVE CJHFT/CJLFT AT DEFAULT VALUES FOR PWF EXAMPLE
  // note: these values would potentially be used to indicate material or component
  //       limits have been exceeded for your specific measurement configuration
/*  CJHFT, 03h/83h: cold-jcn high fault threshold, default 0x7F (bit 7 is sign)
  CJLFT, 04h/84h: cold-jcn low fault threshold, default 0x00) */

  // LEAVE LTXFTX AT DEFAULT VALUES FOR PWF EXAMPLE
  // note: these values would potentially be used to indicate material limits
  //       have been exceeded for your specific thermocouple
/*  LTHFTH, 05h/85h: Linearize temperature high fault thresh MSB (bit 7 is sign)
  LTHFTL, 06h/86h: Linearize temperature high fault thresh LSB
  LTLFTH, 07h/87h: Linearize temperature low fault thresh MSB (bit 7 is sign)
  LTLFTL, 08h/88h: Linearize temperature low fault thresh LSB */
}

void MAX31856::prime(uint8_t CSx, uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT) {
  _cs = CSx;
  _tcType = TC_TYPE;
  _filtFreq = FILT_FREQ;
  _avgMode = AVG_MODE;
  _cMode = CMODE;
  _oneShot = ONE_SHOT;

  this->internal = 0;
  this->external = 0;
  this->fault = 0;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  uint8_t regdat = 0;   // set up paramater to compile register configs
  // set CR0 (REG_CR0)
  regdat = (_cMode | _oneShot | OCFAULT_10MS | CJ_ENABLED | FAULT_AUTO | FAULT_CLR_DEF | _filtFreq);
  regWrite(REG_CR0, 0xFF, regdat); // write data to register
  regdat = (_avgMode | _tcType);
  regWrite(REG_CR1, 0xFF, regdat);
  regdat = (CJ_HIGH_MASK | CJ_LOW_MASK | TC_HIGH_MASK | TC_LOW_MASK);
  regWrite(REG_MASK, 0x3F, regdat);
}

void MAX31856::read() {
  uint16_t cj = 0;
  uint32_t ltc = 0;
  SPI.beginTransaction(_settings);
  digitalWrite(_cs, LOW);
  SPI.transfer(REG_CJTH | 0x00);
  cj |= (uint16_t)SPI.transfer(0) << 8;
  cj |= SPI.transfer(0);
  ltc |= (uint32_t)SPI.transfer(0) << 16;
  ltc |= (uint32_t)SPI.transfer(0) << 8;
  ltc |= SPI.transfer(0);
  fault = SPI.transfer(0);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();

  cj >>= 2;
  internal = (cj >= (1U << 13) ? cj - (1UL << 14) : cj) * 0.015625;

  int32_t signed_ltc = (int32_t)(ltc >= (1UL << 23) ? ltc - (1UL << 24) : ltc) / 32;
  external = signed_ltc * 0.0078125;
}

double MAX31856::getInternal() {
	return internal;
}

double MAX31856::getExternal() {
	return external;
}

bool MAX31856::isExternalOutOfRange() {
	return (fault & 0x40) != 0;
}

bool MAX31856::isInternalOutOfRange() {
	return (fault & 0x80) != 0;
}

bool MAX31856::isOverUnderVoltage() {
	return (fault & 0x02) != 0;
}

bool MAX31856::isOpen() {
	return (fault & 0x01) != 0;
}

bool MAX31856::hasError() {
	return (fault & 0xC3) != 0;
}

uint8_t MAX31856::regRead(uint8_t RegAdd)
{
  SPI.beginTransaction(_settings);
  digitalWrite(_cs, LOW);           // set pin low to start talking to IC
  // next pack address byte
  // bits 7:4 are 0 for read, register is in bits 3:0... format 0Xh
  SPI.transfer((RegAdd & 0x0F));        // write address
  // then read register data
  uint8_t RegData = SPI.transfer(0x00);     // read register data from IC
  digitalWrite(_cs, HIGH);          // set pin high to end SPI session
<<<<<<< HEAD:src/MAX31856.cpp

=======
  SPI.endTransaction();
>>>>>>> c3d480a01ac9eec1e8e97cb890503aebf003aaca:MAX31856.cpp
  return RegData;
}

void MAX31856::regWrite(uint8_t RegAdd, uint8_t BitMask, uint8_t RegData)
{
  // start by reading original register data (we're only modifying what we need to)
  uint8_t OrigRegData = regRead(RegAdd);

  // calculate new register data... 'delete' old targeted data, replace with new data
  // note: 'BitMask' must be bits targeted for replacement
  // add'l note: this function does NOT shift values into the proper place... they need to be there already
  uint8_t NewRegData = ((OrigRegData & ~BitMask) | (RegData & BitMask));
  SPI.beginTransaction(_settings);
  // now configure and write the updated register value
  digitalWrite(_cs, LOW);             // set pin low to start talking to IC
  // next pack address byte
  // bits 7:4 are 1000b for read, register is in bits 3:0... format 8Xh
  SPI.transfer((RegAdd & 0x0F) | 0x80);     // simple write, nothing to read back
  SPI.transfer(RegData);              // write register data to IC
  digitalWrite(_cs, HIGH);            // set pin high to end SPI session
  SPI.endTransaction();
}
