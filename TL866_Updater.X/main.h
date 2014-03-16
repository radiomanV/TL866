/* 
 * File:   main.h
 * Author: Radioman
 *
 *
 */

#ifndef MAIN_H
#define	MAIN_H




#define _XTAL_FREQ 48000000

//TL866 Ports settings
#define INIT_LATA  (0b00010000)
#define INIT_LATB  (0b00000010)
#define INIT_LATC  (0b00000000)
#define INIT_LATD  (0b00000000)
#define INIT_LATE  (0b00000000)
#define INIT_LATF  (0b00000000)
#define INIT_LATG  (0b00010000)
#define INIT_LATH  (0b00000000)
#define INIT_LATJ  (0b00000000)

#define INIT_TRISA (0b00000000)
#define INIT_TRISB (0b00000001)
#define INIT_TRISC (0b00000000)
#define INIT_TRISD (0b00000000)
#define INIT_TRISE (0b00000000)
#define INIT_TRISF (0b00000000)
#define INIT_TRISG (0b00000000)
#define INIT_TRISH (0b00000000)
#define INIT_TRISJ (0b00000000)

#define LE0 (LATHbits.LATH0)//vpp0
#define LE1 (LATHbits.LATH1)//vpp1
#define LE2 (LATAbits.LATA2)//vcc0
#define LE3 (LATAbits.LATA0)//vcc1
#define LE4 (LATAbits.LATA5)//vcc2
#define LE5 (LATAbits.LATA3)//gnd0
#define LE6 (LATHbits.LATH4)//gnd2
#define LE7 (LATAbits.LATA1)//gnd3
#define OE_VPP (LATGbits.LATG4)//OE_VPP
#define OE_VCC (LATAbits.LATA4)//OE_VCC
#define SR_DAT (LATHbits.LATH2)// shift register data line
#define SR_CLK (LATHbits.LATH3)// shift register clock line (register shift on 1)

#define LED_PIN     (LATCbits.LATC0)
#define LED_ON      1
#define LED_OFF     0

#define INPUT_PIN 1
#define OUTPUT_PIN 0
#define FLAG_FALSE 0
#define FLAG_TRUE 1

#define VERSION_A  1
#define VERSION_CS 2

#define OVER_HEAD   5           //Overhead: <CMD_CODE><LEN><ADDR:3>
#define DATA_SIZE   (USBGEN_EP_SIZE - OVER_HEAD)

#define READ_VERSION        0x00
#define READ_FLASH          0x01
#define WRITE_BOOTLOADER    0x02
#define WRITE_CONFIG        0x03
#define WRITE_INFO          0x04
#define GET_INFO            0x05
//#define DEBUG_PACKET        0x7F
#define GET_PORT_INP        0x80
#define GET_PORT_LAT        0x81
#define GET_PORT_TRIS       0x82
#define SET_PORT_TRIS       0x83
#define SET_PORT_LAT        0x84
#define SET_SHIFTREG        0x85
#define RESET               0xFF

typedef union {
    unsigned char _byte[USBGEN_EP_SIZE]; //For Byte Access

    struct {
        unsigned char CMD;
        unsigned char len;

        union {
            rom far char *pAdr; //Address Pointer

            struct {
                unsigned char low; //Little-endian order
                unsigned char high;
                unsigned char upper;
            };
        } ADR;
        unsigned char data[DATA_SIZE];
    };
} DATA_PACKET;

#endif	/* MAIN_H */

