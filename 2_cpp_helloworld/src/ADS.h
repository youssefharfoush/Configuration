/*
 * ADS.h
 *
 *  Created on: Apr 7, 2015
 *      Author: apple
 */

#ifndef ADS_H_
#define ADS_H_


#include <stdio.h>
#include <Arduino.h>
#include "Definitions.h"


class ADS {


//We can make it a private variable and write a set function --> avoids passing it as a variable
//private: mraa_gpio_context gpio_cs;

public:



    void initialize(int _DRDY, int _RST, int _CS, int _FREQ, boolean _isDaisy);

    //ADS1299 SPI Command Definitions (Datasheet, p35)

    //System Commands
    void WAKEUP(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void STANDBY(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void RESET(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void START(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void STOP(mraa_gpio_context gpio_cs,mraa_spi_context spi );

    //Data Read Commands
    void RDATAC(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void SDATAC(mraa_gpio_context gpio_cs,mraa_spi_context spi );
    void RDATA(mraa_gpio_context gpio_cs,mraa_spi_context spi );

    //Register Read/Write Commands
    byte getDeviceID(mraa_gpio_context gpio_cs,mraa_spi_context spi);
    byte RREG(byte _address,mraa_gpio_context gpio_cs, mraa_spi_context spi);
    void RREGS(byte _address, byte _numRegistersMinusOne, mraa_spi_context spi);
    void printRegisterName(byte _address);
    void WREG(byte _address, byte _value, mraa_spi_context spi);
    void WREGS(byte _address, byte _numRegistersMinusOne, mraa_spi_context spi);
    void printHex(byte _data);
    void updateChannelData(mraa_spi_context spi);

    //SPI Transfer function
    int transfer(byte _data , mraa_spi_context spi);

    //configuration
    int DRDY, CS; 		// pin numbers for DRDY and CS
    int DIVIDER;		// select SPI SCK frequency
    int stat_1, stat_2;    // used to hold the status register for boards 1 and 2
    byte regData [24];	// array is used to mirror register data
    long channelData [16];	// array used when reading channel data board 1+2
    boolean verbose;		// turn on/off Serial feedback
    boolean isDaisy;		// does this have a daisy chain board?

};

#endif /* ADS_H_ */





