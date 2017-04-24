/**
 * @file MAX31856.h
 * This is the library to access the MAX31856
 *
 * Copyright (c) 2015 Kina Smith
 * This software is released under the MIT license. See the attached LICENSE file for details.
 */

#ifndef MAX31856_h
#define MAX31856_h

#include "Arduino.h"
#include <SPI.h>

// Registers
#define REG_CR0     0x00    // Config Reg 0 - See Datasheet, pg 19
#define REG_CR1     0x01    // Config Reg 1 - averaging and TC type
#define REG_MASK    0x02    // Fault mask register (for fault pin)
#define REG_CJHF    0x03    // Cold Jcn high fault threshold, 1 degC/bit
#define REG_CJLF    0x04    // Cold Jcn low fault threshold, 1 degC/bit
#define REG_LTHFTH  0x05    // TC temp high fault threshold, MSB, 0.0625 degC/bit
#define REG_LTHFTL  0x06    // TC temp high fault threshold, LSB
#define REG_LTLFTH  0x07    // TC temp low fault threshold, MSB, 0.0625 degC/bit
#define REG_LTLFTL  0x08    // TC temp low fault threshold, LSB
#define REG_CJTO    0x09    // Cold Jcn Temp Offset Reg, 0.0625 degC/bit
#define REG_CJTH    0x0A    // Cold Jcn Temp Reg, MSB, 0.015625 deg C/bit (2^-6)
#define REG_CJTL    0x0B    // Cold Jcn Temp Reg, LSB
#define REG_LTCBH   0x0C    // Linearized TC Temp, Byte 2, 0.0078125 decC/bit
#define REG_LTCBM   0x0D    // Linearized TC Temp, Byte 1
#define REG_LTCBL   0x0E    // Linearized TC Temp, Byte 0
#define REG_SR      0x0F    // Status Register

// CR0 Configs
#define CMODE_OFF       0x00
#define CMODE_AUTO      0x80
#define ONESHOT_OFF     0x00
#define ONESHOT_ON      0x40
#define OCFAULT_OFF     0x00
#define OCFAULT_10MS    0x10
#define OCFAULT_32MS    0x20
#define OCFAULT_100MS   0x30
#define CJ_ENABLED      0x00
#define CJ_DISABLED     0x08
#define FAULT_AUTO      0x00
#define FAULT_MANUAL    0x04
#define FAULT_CLR_DEF   0x00
#define FAULT_CLR_ALT   0x02
#define CUTOFF_60HZ     0x00
#define CUTOFF_50HZ     0x01

// CR1 Configs
#define AVG_SEL_1SAMP   0x00
#define AVG_SEL_2SAMP   0x20
#define AVG_SEL_4SAMP   0x40
#define AVG_SEL_8SAMP   0x60
#define AVG_SEL_16SAMP  0x80
#define B_TYPE          0x00
#define E_TYPE          0x01
#define J_TYPE          0x02
#define K_TYPE          0x03
#define N_TYPE          0x04
#define R_TYPE          0x05
#define S_TYPE          0x06
#define T_TYPE          0x07

// MASK Configs
#define CJ_HIGH_MASK    0x20
#define CJ_LOW_MASK     0x10
#define TC_HIGH_MASK    0x08
#define TC_LOW_MASK     0x04
#define OV_UV_MASK      0x02
#define OPEN_FAULT_MASK 0x01


class MAX31856 {
  public:
    MAX31856(uint8_t CSx, uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT);

    /**
     * @brief Initializes the module.
     * Initializes SPI and CS pin.
     */
    // void begin();
    //  void prime(uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT);
    //  void prime(uint8_t CSx, uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT);
     void prime(uint8_t CSx, uint8_t TC_TYPE, uint8_t FILT_FREQ, uint8_t AVG_MODE, uint8_t CMODE, uint8_t ONE_SHOT);
    /**
     * @brief Reads all temperatures.
     * @see getInternal()
     * @see getExternal()
     * @see hasError()
     */
    void read();

    /**
     * @brief Gets the last external temperature reading (hot junction).
     * @return The last external temperature reading.
     * @see read()
     */
    double getExternal();

    /**
     * @brief Gets the last internal temperature reading (cold junction).
     * @return The last internal temperature reading.
     * @see read()
     */
    double getInternal();

    /**
     * @brief Checks if external temperature is out of range.
     * @return True if external temperature (hot junction) is out of range.
     */
    bool isExternalOutOfRange();

    /**
     * @brief Checks if internal temperature is out of range.
     * @return True if internal temperature (cold junction) is out of range.
     */
    bool isInternalOutOfRange();

    /**
     * @brief Checks for overvoltage or undervoltage.
     * @return True if there is overvoltage or undervoltage on the thermocouple inputs.
     */
    bool isOverUnderVoltage();

    /**
     * @brief Checks if thermocouple circuit is open.
     * @return True if thermocouple circuit is open.
     */
    bool isOpen();

    /**
     * @brief Checks if there are errors.
     * @return True if any of the following errors is detected: open circuit, overvoltage, undervoltage,
     *         internal temperature out of range or external temperatur out of range.
     * @see isExternalOutOfRange()
     * @see isInternalOutOfRange()
     * @see isOverUnderVoltage()
     * @see isOpen()
     */
    bool hasError();

  private:
    // static SPISettings spiSettings;
    static SPISettings _settings;
    uint8_t _cs;
    uint8_t _tcType, _filtFreq, _avgMode, _cMode, _oneShot;
    double internal;
    double external;
    uint8_t fault;

    uint8_t regRead(uint8_t RegAdd);
    void regWrite(uint8_t RegAdd, uint8_t BitMask, uint8_t RegData);
};

#endif
