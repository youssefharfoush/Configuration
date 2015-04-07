/*
 * ADS.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: apple
 */

#include "ADS.h"
#include "mraa.h"
#include "Definitions.h"


void ADS::initialize(int _DRDY, int _RST, int _CS, int _FREQ, boolean _isDaisy){

	isDaisy = _isDaisy;

	DRDY = _DRDY;

	CS = _CS; // CS= 51 or 53

	int FREQ = _FREQ;

	int RST = _RST;



	delay(50);				// recommended power up sequence requiers Tpor (~32mS)

	//pinMode(RST,OUTPUT);
	//create a GPIO structure/context for RESET
	mraa_gpio_context gpio_reset;
	// Initialize pin 2
	gpio_reset = mraa_gpio_init(RST);
	// Set direction to OUTPUT
	mraa_gpio_dir(gpio_reset, MRAA_GPIO_OUT);



	//pinMode(RST,LOW);
	mraa_gpio_write(gpio_reset, 0);

	delayMicroseconds(4);	// toggle reset pin

	//pinMode(RST,HIGH);
	mraa_gpio_write(gpio_reset, 1);

	delayMicroseconds(20);	// recommended to wait 18 Tclk before using device (~8uS);



    // **** ----- SPI Setup ----- **** //



    // Set direction register for SCK and MOSI pin.

    // MISO pin automatically overrides to INPUT.

    // When the SS pin is set as OUTPUT, it can be used as

    // a general purpose output port (it doesn't influence

    // SPI operations).


	 mraa_spi_context spi;
	 spi = mraa_spi_init(0);

	 //Intel Edison pins initialization
	 int SCK=55;
	 int MOSI=57;
	 int MISO=59;
	 int SS=0;


	 /* Already configured in Edison as INPUT and OUTPUT
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SS, OUTPUT);

    digitalWrite(SCK, LOW);
    digitalWrite(MOSI, LOW);
    digitalWrite(SS, HIGH);

    // set as master and enable SPI

    SPCR |= _BV(MSTR);

    SPCR |= _BV(SPE);

    //set bit order

    SPCR &= ~(_BV(DORD); ////SPI data format is MSB (pg. 25)

	// set data mode

    SPCR = (SPCR & ~SPI_MODE_MASK) | SPI_DATA_MODE; //clock polarity = 0; clock phase = 1 (pg. 8)

    // set clock divider

	switch (FREQ){

		case 8:

			DIVIDER = SPI_CLOCK_DIV_2;

			break;

		case 4:

			DIVIDER = SPI_CLOCK_DIV_4;

			break;

		case 1:

			DIVIDER = SPI_CLOCK_DIV_16;

			break;

		default:

			break;

	}

    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (DIVIDER);  // set SCK frequency

    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | (DIVIDER); // by dividing 16MHz system clock




*/






    // **** ----- End of SPI Setup ----- **** //



    // Initialize the  data ready chip select and reset pins:

	 //Initialize DRDY
	 	mraa_gpio_context gpio_DRDY;
	 	gpio_DRDY = mraa_gpio_init(DRDY);
	 	mraa_gpio_dir(gpio_reset, MRAA_GPIO_IN);



	 //Initialize CS
	 	mraa_gpio_context gpio_CS;
		gpio_CS = mraa_gpio_init(CS);
	 	mraa_gpio_dir(gpio_CS, MRAA_GPIO_OUT);

	 //Set DRDY and CS to HIGH
	 	mraa_gpio_write(gpio_DRDY,HIGH);
	 	mraa_gpio_write(gpio_CS,HIGH);




}

//CHECK IF CHANGES IN ROUTINES WILL REMAIN THE SAME AFTER RETURN, ELSE HAVE TO PASS THE ARGUMENTS BY REFERENCE ??



//System Commands

void ADS::WAKEUP(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_WAKEUP, spi);

	mraa_gpio_write(gpio_cs,HIGH);

	//must wait 4 tCLK cycles before sending another command (Datasheet, pg. 35)
    delayMicroseconds(3);

}




void ADS::STANDBY(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {		// only allowed to send WAKEUP after sending STANDBY

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_STANDBY,spi);

 	mraa_gpio_write(gpio_cs,LOW);

}




void ADS::RESET(mraa_gpio_context gpio_cs,mraa_spi_context spi ) {			// reset all the registers to default settings

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_RESET,spi);

    delayMicroseconds(12);   	//must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)

 	mraa_gpio_write(gpio_cs,LOW);

}




void ADS::START(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {			//start data conversion

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_START,spi);

 	mraa_gpio_write(gpio_cs,LOW);

}




void ADS::STOP(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {			//stop data conversion

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_STOP, spi);

 	mraa_gpio_write(gpio_cs,LOW);

}




void ADS::RDATAC(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_RDATAC, spi);

 	mraa_gpio_write(gpio_cs,LOW);

	delayMicroseconds(3);

}

void ADS::SDATAC(mraa_gpio_context gpio_cs, mraa_spi_context spi ) {

 	mraa_gpio_write(gpio_cs,LOW);

    transfer(_SDATAC, spi);

 	mraa_gpio_write(gpio_cs,LOW);

	delayMicroseconds(3);   //must wait 4 tCLK cycles after executing this command (Datasheet, pg. 37)

}







// Register Read/Write Commands

byte ADS::getDeviceID(mraa_gpio_context gpio_cs , mraa_spi_context spi) {			// simple hello world com check

	byte data = RREG(0x00,gpio_cs, spi);

	if(verbose){						// verbose otuput

		printf("Device ID ");

		printHex(data);

	}

	return data;

}




byte ADS::RREG(byte _address,mraa_gpio_context gpio_cs, mraa_spi_context spi) {		//  reads ONE register at _address

    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address

 	mraa_gpio_write(gpio_cs,LOW); 				//  open SPI

    transfer(opcode1, spi); 					//  opcode1

    transfer(0x00,spi); 					//  opcode2

    regData[_address] = transfer(0x00,spi);//  update mirror location with returned byte

 	mraa_gpio_write(gpio_cs,HIGH); 			//  close SPI

	if (verbose){						//  verbose output

		printRegisterName(_address);

		printHex(_address);

		printf(", ");

		printHex(regData[_address]);

		printf(", ");

		for(byte j = 0; j<8; j++){

			printf("%d " , bitRead(regData[_address], 7-j));

			if(j!=7) printf(", ");

		}



		printf(" \n");

	}

	return regData[_address];			// return requested register value

}




// Read more than one register starting at _address

void ADS::RREGS(byte _address, byte _numRegistersMinusOne, mraa_spi_context spi) {

//	for(byte i = 0; i < 0x17; i++){

//		regData[i] = 0;					//  reset the regData array

//	}

    byte opcode1 = _address + 0x20; 	//  RREG expects 001rrrrr where rrrrr = _address

    digitalWrite(CS, LOW); 				//  open SPI

    transfer(opcode1,spi); 					//  opcode1

    transfer(_numRegistersMinusOne,spi);	//  opcode2

    for(int i = 0; i <= _numRegistersMinusOne; i++){

        regData[_address + i] = transfer(0x00,spi); 	//  add register byte to mirror array

		}

    digitalWrite(CS, HIGH); 			//  close SPI

	if(verbose){						//  verbose output

		for(int i = 0; i<= _numRegistersMinusOne; i++){

			printRegisterName(_address + i);

			printHex(_address + i);

			printf(", ");

			printHex(regData[_address + i]);

			printf(", ");

			for(int j = 0; j<8; j++){

				printf("%d" , bitRead(regData[_address + i], 7-j));

				if(j!=7) printf(", ");

			}

			printf(" \n");

		}

    }



}




void ADS::WREG(byte _address, byte _value, mraa_spi_context spi) {	//  Write ONE register at _address

    byte opcode1 = _address + 0x40; 	//  WREG expects 010rrrrr where rrrrr = _address

    digitalWrite(CS, LOW); 				//  open SPI

    transfer(opcode1,spi);					//  Send WREG command & address

    transfer(0x00,spi);						//	Send number of registers to read -1

    transfer(_value,spi);					//  Write the value to the register

    digitalWrite(CS, HIGH); 			//  close SPI

	regData[_address] = _value;			//  update the mirror array

	if(verbose){						//  verbose output

		printf("Register ");

		printHex(_address);

		printf(" modified.\n");

	}

}




void ADS::WREGS(byte _address, byte _numRegistersMinusOne, mraa_spi_context spi) {

    byte opcode1 = _address + 0x40;		//  WREG expects 010rrrrr where rrrrr = _address

    digitalWrite(CS, LOW); 				//  open SPI

    transfer(opcode1,spi);					//  Send WREG command & address

    transfer(_numRegistersMinusOne,spi);	//	Send number of registers to read -1

	for (int i=_address; i <=(_address + _numRegistersMinusOne); i++){

		transfer(regData[i],spi);			//  Write to the registers

	}

	digitalWrite(CS,HIGH);				//  close SPI

	if(verbose){

		printf("Registers ");

		printHex(_address); printf(" to ");

		printHex(_address + _numRegistersMinusOne);

		printf(" modified");

	}

}







void ADS::updateChannelData(mraa_spi_context spi){

	byte inByte;

	int nchan=8;  //assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.

	digitalWrite(CS, LOW);				//  open SPI



	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE

	for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 1 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])

		inByte = transfer(0x00,spi);

		stat_1 = (stat_1<<8) | inByte;

	}



	for(int i = 0; i<8; i++){

		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks

			inByte = transfer(0x00,spi);

			channelData[i] = (channelData[i]<<8) | inByte;

		}

	}



	if (isDaisy) {

		nchan = 16;

		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE

		for(int i=0; i<3; i++){			//  read 3 byte status register from ADS 2 (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])

			inByte = transfer(0x00,spi);

			stat_2 = (stat_1<<8) | inByte;

		}



		for(int i = 8; i<16; i++){

			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks

				inByte = transfer(0x00,spi);

				channelData[i] = (channelData[i]<<8) | inByte;

			}

		}

	}



	digitalWrite(CS, HIGH);				//  close SPI



	//reformat the numbers

	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment

		if(bitRead(channelData[i],23) == 1){

			channelData[i] |= 0xFF000000;

		}else{

			channelData[i] &= 0x00FFFFFF;

		}

	}

}






//read data

void ADS::RDATA(mraa_gpio_context gpio_cs, mraa_spi_context spi) {				//  use in Stop Read Continuous mode when DRDY goes low

	byte inByte;

	stat_1 = 0;							//  clear the status registers

	stat_2 = 0;

	int nchan = 8;	//assume 8 channel.  If needed, it automatically changes to 16 automatically in a later block.

	digitalWrite(CS, LOW);				//  open SPI

	transfer(_RDATA,spi);



	// READ CHANNEL DATA FROM FIRST ADS IN DAISY LINE

	for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])

		inByte = transfer(0x00,spi);

		stat_1 = (stat_1<<8) | inByte;

	}



	for(int i = 0; i<8; i++){

		for(int j=0; j<3; j++){		//  read 24 bits of channel data from 1st ADS in 8 3 byte chunks

			inByte = transfer(0x00,spi);

			channelData[i] = (channelData[i]<<8) | inByte;

		}

	}



	if (isDaisy) {

		nchan = 16;



		// READ CHANNEL DATA FROM SECOND ADS IN DAISY LINE

		for(int i=0; i<3; i++){			//  read 3 byte status register (1100+LOFF_STATP+LOFF_STATN+GPIO[7:4])

			inByte = transfer(0x00,spi);

			stat_2 = (stat_1<<8) | inByte;

		}



		for(int i = 8; i<16; i++){

			for(int j=0; j<3; j++){		//  read 24 bits of channel data from 2nd ADS in 8 3 byte chunks

				inByte = transfer(0x00,spi);

				channelData[i] = (channelData[i]<<8) | inByte;

			}

		}

	}



	for(int i=0; i<nchan; i++){			// convert 3 byte 2's compliment to 4 byte 2's compliment

		if(bitRead(channelData[i],23) == 1){

			channelData[i] |= 0xFF000000;

		}else{

			channelData[i] &= 0x00FFFFFF;

		}

	}





}










// String-Byte converters for RREG and WREG

void ADS::printRegisterName(byte _address) {

    if(_address == ID){

        printf("ID, "); //the "F" macro loads the string directly from Flash memory, thereby saving RAM

    }

    else if(_address == CONFIG1){

        printf("CONFIG1, ");

    }

    else if(_address == CONFIG2){

        printf("CONFIG2, ");

    }

    else if(_address == CONFIG3){

        printf("CONFIG3, ");

    }

    else if(_address == LOFF){

        printf("LOFF, ");

    }

    else if(_address == CH1SET){

        printf("CH1SET, ");

    }

    else if(_address == CH2SET){

        printf("CH2SET, ");

    }

    else if(_address == CH3SET){

        printf("CH3SET, ");

    }

    else if(_address == CH4SET){

        printf("CH4SET, ");

    }

    else if(_address == CH5SET){

        printf("CH5SET, ");

    }

    else if(_address == CH6SET){

        printf("CH6SET, ");

    }

    else if(_address == CH7SET){

        printf("CH7SET, ");

    }

    else if(_address == CH8SET){

        printf("CH8SET, ");

    }

    else if(_address == BIAS_SENSP){

        printf("BIAS_SENSP, ");

    }

    else if(_address == BIAS_SENSN){

        printf("BIAS_SENSN, ");

    }

    else if(_address == LOFF_SENSP){

        printf("LOFF_SENSP, ");

    }

    else if(_address == LOFF_SENSN){

        printf("LOFF_SENSN, ");

    }

    else if(_address == LOFF_FLIP){

        printf("LOFF_FLIP, ");

    }

    else if(_address == LOFF_STATP){

        printf("LOFF_STATP, ");

    }

    else if(_address == LOFF_STATN){

        printf("LOFF_STATN, ");

    }

    else if(_address == GPIO){

        printf("GPIO, ");

    }

    else if(_address == MISC1){

        printf("MISC1, ");

    }

    else if(_address == MISC2){

        printf("MISC2, ");

    }

    else if(_address == CONFIG4){

        printf("CONFIG4, ");

    }

}




//SPI communication methods

int ADS::transfer(byte _data ,mraa_spi_context spi ) {

	int status=-1;
    uint8_t data_transfer =  _data ;
    status = mraa_spi_write( spi,data_transfer );

    return status;

}




// Used for printing HEX in verbose feedback mode

void ADS::printHex(byte _data){

	printf("0x");

    if(_data < 0x10) printf("0");

    printf("%02x",_data);

}




//-------------------------------------------------------------------//

//-------------------------------------------------------------------//

//-------------------------------------------------------------------//




