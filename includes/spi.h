#include <util/delay.h>
#include "bit.h"

/* Initializes master mode */
void spiInitMaster();

/* Initializes slave mode */
void spiInitSlave();

/* Transfers command and data, fills out return*/
void spiTransfer(char command, unsigned char *data, unsigned char *ret, int length);

/* Writes data, discards response */
void spiWrite(char command, unsigned char *data, int length);

/* Reads data, sends nulls*/
void spiRead(char command, unsigned char *ret, int length);

void spiInitMaster()
{
	// Set DDRB to have MOSI, SCK, and SS as output and MISO as input
	 DDRB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB7);
	 DDRB &= ~(1 << DDB6);
	// Set SPCR register to enable SPI, enable master, and use SCK frequency
	// of fosc/16  (pg. 168)
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);//|(1 << SPIE); (interupt)
	
	// global interups enabled
	//sei();
}

void spiInitSlave()
{
    //DDRB = 0x4F; PORTB = 0xB0;
    DDRB = (1<<6);     //MISO as OUTPUT
    
    //SPCR = 0xC0;
    SPCR = (1 << SPE) | (1 << SPIE) | (1 << SPR0);
    
    // global interups enabled
    //SREG |= 0x80;
}

//Function to send and receive data for both master and slave
char spiSend (unsigned char data)
{
    // Load data into the buffer
    SPDR = data;
 
    //Wait until transmission complete
    while(!(SPSR & (1<<SPIF)));
    
    // Return received data
    return(SPDR);
}

void spiTransfer(char command, unsigned char *data, unsigned char *ret, int length)
{
	// set SS low
	PORTB = SetBit(PORTB,4,0);

	spiSend(command);
	
	PORTB = SetBit(PORTB,4,1);
    
    int i;
    for(i = 0; i < length; i++)
    {
        uoutSend("Send byte! \n\r");
    
    	PORTB = SetBit(PORTB,4,0);
        ret[i] = spiSend(data[i]);
	    PORTB = SetBit(PORTB,4,1);
    }

	// set SS high

}

void spiWrite(char command, unsigned char *data, int length)
{
	// set SS low
	PORTB = SetBit(PORTB,4,0);

	spiSend(command);
    
    int i;
    for(i = 0; i < length; i++)
    {
        spiSend(data[i]);
    }

	// set SS high
	PORTB = SetBit(PORTB,4,1);

}

void spiWriteByte(char command, unsigned char data)
{
    spiWrite(command, &data, 1);
}

void spiRead(char command, unsigned char *ret, int length)
{
	// set SS low
	PORTB = SetBit(PORTB,4,0);

	spiSend(command);
    
    int i;
    for(i = 0; i < length; i++)
    {
        //uoutSend("Send byte! \n\r");
        
        ret[i] = spiSend(0x00);
    }

	// set SS high
	PORTB = SetBit(PORTB,4,1);
}