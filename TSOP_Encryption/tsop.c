/*
* tsop.c
*
* Created: 22.02.2014 07:22:55
*  Author: radioman
*/

#define F_CPU 4800000UL //Internal 4.8Mhz clock

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

//Extracted xor table from TL866 firmware at 0x0196AA
const unsigned char xortable[] PROGMEM =
{
	0x48, 0x0D, 0x11, 0xB6, 0xA4, 0x76, 0xA0, 0xF6, 0x66, 0xE9, 0x55, 0xE5, 0xAC, 0x20, 0xDB, 0x16,
	0x0C, 0x65, 0x1F, 0x53, 0x7A, 0xEC, 0x55, 0x8F, 0x6C, 0x65, 0x3D, 0x1C, 0x3C, 0xD2, 0xA4, 0x1F,
	0xCD, 0x96, 0x2D, 0x40, 0x71, 0xCF, 0x51, 0xA6, 0x0A, 0xE6, 0xA5, 0x22, 0x70, 0x65, 0x5B, 0xAA,
	0x13, 0x08, 0x0C, 0x0A, 0xE2, 0x16, 0xB5, 0xD6, 0x6B, 0xF1, 0xFF, 0xA3, 0x43, 0xEC, 0xC3, 0x76,
	0xA8, 0x5E, 0xCD, 0x7D, 0x65, 0xF5, 0xE0, 0xFD, 0xF6, 0x4C, 0xFB, 0x8C, 0xED, 0xBF, 0xDB, 0x80,
	0x94, 0x7E, 0xC1, 0xA7, 0xD4, 0xE0, 0x75, 0x39, 0x55, 0xFA, 0x8B, 0x0A, 0xE7, 0x71, 0xE5, 0x04,
	0x21, 0x8C, 0x79, 0xD5, 0x47, 0x8D, 0x54, 0xE5, 0x71, 0x42, 0xDF, 0x8B, 0xEA, 0xD7, 0x62, 0x80,
	0xD7, 0xEE, 0xC6, 0x94, 0x19, 0xF1, 0x9F, 0x9F, 0x74, 0xA7, 0x6A, 0xBB, 0xF0, 0x06, 0x12, 0xB0,
	0x7F, 0x49, 0xBA, 0xB0, 0xE1, 0x42, 0xB6, 0x45, 0xC5, 0xF0, 0xDA, 0x87, 0x31, 0x54, 0xF8, 0x92,
	0x23, 0x81, 0xA4, 0x37, 0x79, 0xF3, 0x3A, 0xF2, 0x0E, 0x21, 0x23, 0x1C, 0x26, 0x55, 0x53, 0x62,
	0x0C, 0xBC, 0x16, 0x76, 0xF9, 0xB9, 0x0D, 0x04, 0x38, 0x7F, 0x73, 0xE8, 0x88, 0xDF, 0xA5, 0x9E,
	0xC2, 0x5E, 0xE2, 0xFA, 0xBB, 0x8B, 0x4F, 0x19, 0x6C, 0x90, 0x3E, 0x97, 0x50, 0x06, 0xAF, 0x02,
	0x0E, 0x0E, 0x17, 0x8F, 0x58, 0x9D, 0x61, 0x0C, 0x13, 0x17, 0x33, 0x16, 0xB8, 0x20, 0x72, 0x8C,
	0xFA, 0xAF, 0x08, 0x44, 0xA9, 0x63, 0xE5, 0xFC, 0xD6, 0x1B, 0x43, 0x93, 0x38, 0xC1, 0x2F, 0x79,
	0xCE, 0x66, 0x45, 0x64, 0xC6, 0x94, 0xBB, 0x44, 0x9E, 0xDF, 0xA1, 0x7A, 0x89, 0xBE, 0x66, 0x44,
	0x14, 0x99, 0x9F, 0x7C, 0x09, 0x23, 0x04, 0x83, 0x93, 0xEA, 0xBB, 0x78, 0xA4, 0x2D, 0xDA, 0xAD
};

/*Constant response packet. This packet structure is simple:
1.Xor bytes from offset 4 to 35(0x74 to 0xBA) with byte at offset 0 (0x57)
2.Compute a checksum of first 36 bytes; the result must match the last byte (0x7D). If this checksum doesn't match then the TL866 report "V0 is illegal"

Actually the TL866 firmware will check if the first 8 bytes are: 51 33 51 00 c8 9d d4 3e ; if yes then return FAKE!
Now what happen if we put 37 bytes of '0'? well xor zero by zero equal zero and checksum of an infinite number of zeros is zero! and this will pass the genuine check. Very weak algorithm.
But we keep this as original chip does. Thanks Gerard for your captures.
*/
const unsigned char response[] PROGMEM =
{
	0x57, 0x33, 0x57, 0x00, 0x74, 0x87, 0x75, 0xC5, 0xE9, 0xE6, 0xF5, 0xEE, 0x95, 0x12, 0x5B, 0x66,
	0x1F, 0x0A, 0x68, 0xFC, 0x8A, 0x80, 0x3B, 0xFD, 0x72, 0x01, 0x7E, 0x38, 0x2B, 0x4D, 0xC3, 0x43,
	0xC7, 0xA5, 0x9A, 0xBA, 0x7D
};


//Data line port defines
#define DATA_LINE_DDR		DDRB
#define DATA_LINE_OUT_PORT	PORTB
#define DATA_LINE_IN_PORT	PINB
#define DATA_LINE_PIN		4


//clock line port defines
#define CLOCK_LINE_DDR		DDRB
#define CLOCK_LINE_PORT		PINB
#define CLOCK_LINE_PIN		3

//helper macros
#define data_line		(DATA_LINE_IN_PORT &(1<<DATA_LINE_PIN))
#define clock_line		(CLOCK_LINE_PORT &(1<<CLOCK_LINE_PIN))
#define data_line_high		(DATA_LINE_OUT_PORT |= (1<<DATA_LINE_PIN))
#define data_line_low		(DATA_LINE_OUT_PORT &= ~(1<<DATA_LINE_PIN))
#define set_data_line_out	(DATA_LINE_DDR |=(1<<DATA_LINE_PIN))
#define set_data_line_in	(DATA_LINE_DDR &= ~(1<<DATA_LINE_PIN))

//machine state
#define STATE_RECEIVE		0
#define STATE_TRANSMIT		1


unsigned char buffer[47];//Receive/Send buffer
volatile unsigned char state_machine;//keep the machine state
unsigned char bit;
unsigned char byte;

static inline unsigned int crc16_ccitt(unsigned char* data, unsigned int len);
static inline void spi_receive();
static inline void spi_send();
static inline void check();

//Main routine
int main(void)
{
	DATA_LINE_OUT_PORT |=(1<<DATA_LINE_PIN);//pull-up on data line
	CLOCK_LINE_PORT |=(1<<CLOCK_LINE_PIN);//pull-up on clock line
	PCMSK |=(1<<CLOCK_LINE_PIN);//set PCMSK clock line external interrupt mask
	GIMSK|=(1<<PCIE);//activate external sense interrupts
	bit=0;
	byte=0;
	state_machine=STATE_RECEIVE;//we start in receive mode
	_delay_ms(5);//wait for lines to settle. 
	sei();//enable interrupts
	while(1)//just nothing here. From now all tasks are interrupt driven.
	{
	}
}


//CRC16_CCITT routine
unsigned int crc16_ccitt(unsigned char* data, unsigned int len)
{
	unsigned int crc=0;
	while(len--)
	{
		crc  = (unsigned char)(crc >> 8) | (crc << 8);
		crc ^= *data++;
		crc ^= (unsigned char)(crc & 0xFF) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xFF) << 4) << 1;
	}
	return crc;
}


//Simple interrupt routine handler
ISR(PCINT0_vect)
{
	if(clock_line)//If clock line was changed from 0 to 1 then we call receive/send routines
	{
		if(!state_machine)
		spi_receive();
		else
		spi_send();
	}
	
}

//Receive 10 bytes routine. Driven by clock line
static inline void spi_receive()
{
	buffer[byte]<<=1;
	if(data_line)
	buffer[byte]|=1;
	bit++;
	bit &=7;
	if(!bit)
	{
		byte++;
		if(byte==10)
		{
			byte=0;
			set_data_line_out;//switch the data line in output mode
			check();//check and prepare transmit buffer
			if (buffer[byte] & 0x80)//put the first bit to data line
			data_line_high;
			else
			data_line_low;
			buffer[byte]<<=1;
			bit++;
			state_machine=STATE_TRANSMIT;
		}
	}
}

//Send 47 bytes routine. Driven by clock line.
static inline void spi_send()
{
	if (buffer[byte] & 0x80)
	data_line_high;
	else
	data_line_low;
	buffer[byte]<<=1;
	bit++;
	bit &=7;
	if(!bit)
	{
		byte++;
		if(byte==47)
		{
			byte=0;
		}
	}
}

//check the received data
static inline void check()
{
	unsigned char i,index;
	//swap byte 2 by 8 and 4 by 9
	i=buffer[8];
	buffer[8]=buffer[2];
	buffer[2]=i;
	i=buffer[9];
	buffer[9]=buffer[4];
	buffer[4]=i;
	//calculate crc16_ccitt of the first 10 bytes. If they match then we change the state machine in transmit mode.
	if(crc16_ccitt(buffer,8)==((buffer[9] << 8) | buffer[8]))
	{
		index=(buffer[0] & buffer[7]);//The starting index for byte xoring is byte 0 AND 7
		for(i=0;i<10;i++)
		{
			buffer[i] ^=pgm_read_byte(&xortable[index++]);//xor first 10 bytes against a xortable
			index &=0xFF;//index is incremented modulo 256
		}
		for(i=0;i<37;i++)
		{
			buffer[i+10]=pgm_read_byte(&response[i]);//copy the next predefined 37 bytes from flash to the SRAM buffer
		}
	}
}


