/* 
 * File:   main.c
 * Author: Radioman
 *
 * Created on 09 aprilie 2013, 07:20
 */


#include <USB/usb.h>
#include <USB/usb_function_generic.h>

#include "HardwareProfile.h"
#include "config_bits.h"
#include <delays.h>
#include <string.h>
#include <stdlib.h>


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

#define LE0     (LATHbits.LATH0)//vpp0
#define LE1     (LATHbits.LATH1)//vpp1
#define LE2     (LATAbits.LATA2)//vcc0
#define LE3     (LATAbits.LATA0)//vcc1
#define LE4     (LATAbits.LATA5)//vcc2
#define LE5     (LATAbits.LATA3)//gnd0
#define LE6     (LATHbits.LATH4)//gnd2
#define LE7     (LATAbits.LATA1)//gnd3
#define OE_VPP  (LATGbits.LATG4)//OE_VPP
#define OE_VCC  (LATAbits.LATA4)//OE_VCC
#define SR_DAT  (LATHbits.LATH2)//shift register data line
#define SR_CLK  (LATHbits.LATH3)//shift register clock line (register shift on 1)

#define LED_PIN (LATCbits.LATC0)
#define LED_ON      1
#define LED_OFF     0

#define INPUT_PIN 1
#define OUTPUT_PIN 0
#define FLAG_FALSE 0
#define FLAG_TRUE 1

#define VERSION_A  1
#define VERSION_CS 2

#define READ_VERSION        0x00
#define READ_FLASH          0x01
#define WRITE_BOOTLOADER    0x02
#define WRITE_CONFIG        0x03
#define WRITE_INFO          0x04
#define GET_INFO            0x05
#define GET_PORT_INP        0x80
#define GET_PORT_LAT        0x81
#define GET_PORT_TRIS       0x82
#define SET_PORT_TRIS       0x83
#define SET_PORT_LAT        0x84
#define SET_SHIFTREG        0x85
#define RESET               0xFF

typedef union {
    unsigned char _byte[USBGEN_EP_SIZE];

    struct {
        unsigned char CMD;
        unsigned char len;
        rom far char *pAdr;
        unsigned char data[USBGEN_EP_SIZE - 5];
    };
} DATA_PACKET;

extern void _startup(void);
void high_ISR(void);
void low_ISR(void);


/**********************************************REMAPPED VECTORS********************************************************/
#define REMAPPED_RESET_VECTOR_ADDRESS           0x1800
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1808
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1818

#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS

void _reset(void) {
    _asm goto _startup _endasm
}

#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS

void Remapped_High_ISR(void) {
    _asm goto high_ISR _endasm
}
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS

void Remapped_Low_ISR(void) {
    _asm goto low_ISR _endasm
}

#pragma code HIGH_INTERRUPT_VECTOR = 0x08

void High_ISR(void) {
    _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code LOW_INTERRUPT_VECTOR = 0x18

void Low_ISR(void) {
    _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code
/*************************************END REMAPPED VECTORS********************************************************/


#define DEVICE_TYPE_LOCATION 0x11B4 //This ROM location will tell us the device type
#define DEVICE_COPY_PROTECT 0x1FFF9 //Copy protect config byte location

//ROM constants
#pragma romdata _signature = 0x1FBFC
const rom unsigned char sigbytes[] = {0x55, 0xAA, 0xA5, 0x5A};
#pragma romdata

far rom unsigned char devcode[] = {'C', 'o', 'd', 'e', 'd', 'u', 'm', 'p'};
far rom unsigned char serial[] = {
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0'
};

far rom unsigned char A_Table[] = {
    0xA4, 0x1E, 0x42, 0x8C, 0x3C, 0x76, 0x14, 0xC7, 0xB8, 0xB5, 0x81, 0x4A, 0x13, 0x37, 0x7C, 0x0A,
    0xFE, 0x3B, 0x63, 0xC1, 0xD5, 0xFD, 0x8C, 0x39, 0xD1, 0x1F, 0x22, 0xC7, 0x7F, 0x4D, 0x2F, 0x15,
    0x71, 0x21, 0xF9, 0x25, 0x33, 0x44, 0x92, 0x93, 0x80, 0xD7, 0xAB, 0x1B, 0xB6, 0x11, 0xA9, 0x5A,
    0x88, 0x29, 0xFB, 0xD9, 0xF3, 0x76, 0xAA, 0x47, 0x73, 0xD5, 0x31, 0x06, 0x76, 0x4B, 0x90, 0xEA,
    0x11, 0xEB, 0x9C, 0x3D, 0xF2, 0xFA, 0x99, 0x06, 0x96, 0x52, 0x0A, 0x8A, 0xBC, 0x04, 0xC8, 0x14,
    0x19, 0x41, 0x52, 0xF2, 0x4D, 0x7B, 0x64, 0xC0, 0x16, 0xC7, 0xCB, 0xE9, 0xC3, 0x86, 0x77, 0x6A,
    0xEC, 0x44, 0xD2, 0xD9, 0x61, 0xE0, 0x50, 0xA6, 0x60, 0xED, 0x47, 0xA2, 0x0B, 0x59, 0x02, 0xBD,
    0x18, 0x4C, 0x11, 0x14, 0xCB, 0x53, 0xE2, 0x2B, 0x21, 0xBE, 0x96, 0x76, 0x4F, 0x47, 0x0D, 0x1F,
    0x6A, 0xF4, 0x43, 0x03, 0x68, 0x3E, 0xE0, 0xFE, 0x47, 0x72, 0x0A, 0x68, 0x8C, 0x58, 0x7E, 0xDF,
    0xEF, 0x13, 0xDF, 0x47, 0x55, 0x48, 0x4D, 0x10, 0xFE, 0x82, 0x3A, 0xB7, 0x00, 0xD5, 0x79, 0x90,
    0xF4, 0xC2, 0x98, 0xC2, 0xEF, 0x5B, 0x70, 0x93, 0xB4, 0xA7, 0xFA, 0xE6, 0x27, 0x48, 0x65, 0x01,
    0x05, 0x5B, 0x65, 0x94, 0xD3, 0xA0, 0xCD, 0xF7, 0x14, 0xDB, 0x60, 0xB4, 0xBF, 0x7A, 0xE4, 0x45,
    0xF0, 0x77, 0x79, 0x1F, 0xDE, 0x80, 0x29, 0xEF, 0x0D, 0x56, 0xC0, 0x23, 0xC5, 0x73, 0xDE, 0xAC,
    0xC2, 0xEF, 0x4A, 0x02, 0x2D, 0xA4, 0x89, 0x69, 0xCB, 0x91, 0xB0, 0x74, 0x75, 0x7C, 0x76, 0xC7,
    0xC8, 0xDB, 0x8D, 0x20, 0x1D, 0xF5, 0x33, 0x99, 0xBB, 0x45, 0x04, 0x27, 0x4C, 0x1F, 0x12, 0x67,
    0x8E, 0x96, 0x37, 0x9A, 0x4B, 0x9C, 0xAA, 0xED, 0x8B, 0x6B, 0xD1, 0xFF, 0x08, 0x24, 0x56, 0x9D
};

far rom unsigned char CS_Table[] = {
    0x0B, 0x08, 0x07, 0x18, 0xEC, 0xC7, 0xDF, 0x8C, 0xD6, 0x76, 0xCE, 0x10, 0x9F, 0x61, 0x7C, 0xF5,
    0x61, 0x09, 0xFB, 0x59, 0xD0, 0x24, 0xB4, 0x4F, 0xCA, 0xE4, 0xA1, 0x3A, 0x30, 0x7C, 0xBD, 0x7A,
    0xF5, 0xE1, 0xB9, 0x4B, 0x74, 0xCD, 0xF1, 0xE9, 0x07, 0x0A, 0x9E, 0xF9, 0xD5, 0xED, 0x4D, 0x24,
    0xEB, 0x21, 0x90, 0x05, 0x8F, 0xA5, 0xF3, 0x45, 0xD0, 0x18, 0x31, 0x04, 0x62, 0x35, 0xA8, 0x7B,
    0xA9, 0x9A, 0x0B, 0xE0, 0x14, 0xCD, 0x57, 0x8A, 0xAC, 0x80, 0x08, 0x56, 0xED, 0x14, 0x8C, 0x49,
    0xD4, 0x5D, 0xF8, 0x77, 0x39, 0xA5, 0xFA, 0x23, 0x5F, 0xF3, 0x0E, 0x27, 0xCA, 0x8D, 0xF5, 0x97,
    0x50, 0xBB, 0x64, 0xA1, 0x73, 0xCE, 0xF9, 0xB7, 0xEE, 0x61, 0x72, 0xF1, 0x8E, 0xDF, 0x21, 0xAC,
    0x43, 0x45, 0x9B, 0x78, 0x77, 0x29, 0xB1, 0x31, 0x9E, 0xFC, 0xA1, 0x6B, 0x0F, 0x8C, 0x8D, 0x13,
    0x12, 0xCC, 0x2B, 0x54, 0x3A, 0xD8, 0xBF, 0xB8, 0xF5, 0x34, 0x46, 0x90, 0x61, 0x54, 0xF4, 0x95,
    0x61, 0x62, 0xE1, 0xCF, 0xF1, 0x3B, 0x00, 0xB6, 0xB6, 0xBB, 0x50, 0x98, 0xD9, 0x3A, 0x56, 0x3A,
    0x16, 0x56, 0xCA, 0xC2, 0x10, 0xF3, 0x91, 0xD4, 0xE8, 0x81, 0xEB, 0xFC, 0x0D, 0x7E, 0xEE, 0x4C,
    0x56, 0x3B, 0x33, 0x46, 0x4E, 0xE2, 0xCF, 0xFC, 0xCF, 0xB8, 0x84, 0x75, 0xD2, 0xA0, 0x39, 0x53,
    0x85, 0xE1, 0xA8, 0xB3, 0x9E, 0x28, 0x57, 0x55, 0xEF, 0xD1, 0xC9, 0xFD, 0x3B, 0x62, 0xF5, 0x18,
    0x49, 0x58, 0xF7, 0xA3, 0x36, 0x27, 0x06, 0x49, 0x0F, 0x7C, 0xA6, 0xCB, 0xA0, 0xC5, 0x1E, 0xA5,
    0x86, 0xF3, 0x2D, 0xEF, 0x8C, 0x7E, 0xF9, 0x81, 0x34, 0xAA, 0x48, 0x5A, 0x93, 0x0A, 0xF2, 0x43,
    0x62, 0x42, 0x97, 0xAF, 0x53, 0x10, 0x8D, 0xE6, 0xA1, 0x8E, 0x1C, 0x62, 0xEB, 0xB1, 0xEE, 0x79
};

far rom unsigned char config_bytes[] = {
    0xA8, 0xF3, 0x05, 0xFF, 0xF8, 0xF6, 0xFF, 0xFF
};


//RAM buffers
#pragma udata big_buffer
unsigned char buffer[0x400]; //1Kbyte data buffer
#pragma udata USB_VARIABLES=0x500
DATA_PACKET rxdatapacket; //USB receive buffer
unsigned char txdatapacket[USBGEN_EP_SIZE]; //USB transmit buffer
#pragma udata CONTEXT_SWITCH=0x700
unsigned long int reset_switch; //Here is the RAM context switch location
#pragma udata


//Used function prototypes
unsigned short crc16(unsigned char *data, unsigned short len);
void decrypt_serial(unsigned char version);
void encrypt_serial(unsigned char version);
void FlashErase(unsigned long address);
void WriteBlock(unsigned long address, unsigned char* buffer);
void WriteBuffer(unsigned long address);
void WriteBootloader(unsigned char version);
void WriteConfig(unsigned char version);
void WriteInfo(unsigned char *info_ptr);
void load_shiftregister(unsigned char value);
void config_io();
void ReadProgMem(void);
void ProcessIo(void);

//Global variables
const far rom unsigned short* crc16Table = (const far rom void*) 0x1FD50; //Pointer to crc16 table in ROM
const far rom unsigned short* infoTable = (const far rom void*) 0x1FD00; //Pointer to encrypted info area (80bytes) in ROM
USB_HANDLE USBOutHandle;
USB_HANDLE USBInHandle;
volatile WORD led_cnt;
unsigned char info[80];

#pragma interruptlow high_ISR

void high_ISR(void) {
#if defined(USB_INTERRUPT)
    // Perform USB device tasks
    USBDeviceTasks();
#endif
}

#pragma interruptlow low_ISR

void low_ISR(void) {
}

void DeviceReset(void) {
    UCONbits.USBEN = 0;
    Delay10KTCYx(255);
    Delay10KTCYx(255);
    Reset();
}

// crc16 CCITT

unsigned short crc16(unsigned char *data, unsigned short len) {
    unsigned short crc = 0;
    while (len--)
        crc = ((crc >> 8) ^ crc16Table[(crc ^ *data++) & 0xFF]);
    return crc;
}

//decrypt info array

void decrypt_serial(unsigned char version) {
    int i;
    unsigned char index;
    const far rom unsigned char* xorTableptr = (version == VERSION_A ? &A_Table[0] : &CS_Table[0]);
    //Step 1, xoring each element from table with a random value from xortable. Starting index is 0x0A. Index is incremented modulo 256
    index = 0x0A;
    for (i = 0; i < sizeof (info); i++) {
        info[i] ^= xorTableptr[index++];
    }
    //Step 2 right rotate the whole array by 3 bits.
    index = info[sizeof (info) - 1] << 5;
    for (i = sizeof (info) - 1; i > 0; i--) {
        info[i] = info[i] >> 3 & 0x1F | info[i - 1] << 5;
    }
    info[0] = info[0] >> 3 & 0x1F | index;
    //Step 3, descrambling data; we put each element in the right position. At the end we have the decrypted serial and devcode ;)
    for (i = 0; i < sizeof (info) / 2; i += 4) {
        index = info[i];
        info[i] = info[sizeof (info) - i - 1];
        info[sizeof (info) - i - 1] = index;
    }
}

//encrypt the info array, ready to be inserted in TL866 firmware.

void encrypt_serial(unsigned char version) {
    int i;
    unsigned char index;
    unsigned short crc;
    const far rom unsigned char* xorTableptr = (version == VERSION_A ? A_Table : CS_Table);

    //Fill the info array[32-77] with random values.
    for (i = 32; i < 78; i++) {
        info[i] = (unsigned char) rand() % 0x100;
    }
    info[34] = 0;
    for (i = 5; i < 34; i++)
        info[34] += info[i];
    crc = crc16(info, sizeof (info) - 2);
    info[sizeof (info) - 1] = crc >> 8;
    info[sizeof (info) - 2] = crc & 0xFF;

    /*Data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
     So we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
     */
    for (i = 0; i < sizeof (info) / 2; i += 4) {
        index = info[i];
        info[i] = info[sizeof (info) - i - 1];
        info[sizeof (info) - i - 1] = index;
        //printf("(%d-%d),",i,s-i-1);
    }
    /*
     * Step 2, Left rotate the whole array by 3 bits.
     */
   index = info[0] >> 5;
    for (i = 0; i < sizeof (info) - 1; i++) {
        info[i] = info[i] << 3 & 0xF8 | info[i + 1] >> 5;
    }
    info[sizeof (info) - 1] = info[sizeof (info) - 1] << 3 & 0xF8 | index;

    //Last step, xoring each info table value with a random number from xortable. The start index in this table is 0x0A. Index is incremented modulo 256
    index = 0x0A;
    for (i = 0; i < sizeof (info); i++) {
        info[i] ^= xorTableptr[index++];
    }
}

//Erase flash page

void FlashErase(unsigned long address) {
    TBLPTR = address;
    EECON1 = 0b00010100;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    EECON1bits.WREN = 0;
    INTCONbits.GIE = 1;
}

//Write a 64 byte block of data

void WriteBlock(unsigned long address, unsigned char* buffer) {
    unsigned char i;
    TBLPTR = address;
    EECON1 = 0b00000100;
    for (i = 0; i < 64; i++) {
        TABLAT = buffer[i];
        _asm tblwtpostinc _endasm
    }
    TBLPTR = address;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    EECON1bits.WREN = 0;
    INTCONbits.GIE = 1;
}


//Write 1Kbyte buffer to flash

void WriteBuffer(unsigned long address) {

    unsigned char i;
    unsigned char *buffer_ptr = &buffer[0];
    for (i = 0; i < 16; i++) {
        WriteBlock(address, buffer_ptr);
        buffer_ptr += 64;
        address += 64;
    }
}

//Rewrite bootloader

void WriteBootloader(unsigned char version) {
    unsigned char i;
    unsigned long addr = 0;
    WriteConfig(version);
    for (i = 0; i < 6; i++) {
        memcpypgm2ram(buffer, (const far rom void*) addr, 0x400); //copy 1024bytes from flash to buffer
        if (i == 4) {
            buffer[0x1B4] = version;
        }
        FlashErase(addr); //erase data block
        WriteBuffer(addr); //write data block
        addr += 0x400; //next data block
    }

}

//Rewrite config area

void WriteConfig(unsigned char version) {
    unsigned char t;
    unsigned long addr = 0x1FC00;
    const far rom unsigned char* xorTableptr = (version == VERSION_A ? &A_Table[0] : &CS_Table[0]);
    memcpypgm2ram(buffer, (const far rom void*) addr, 0x400); //copy 1024bytes from flash to buffer
    memcpypgm2ram(&t, (const far rom void*) DEVICE_TYPE_LOCATION, 1);
    memcpypgm2ram(&info[0], infoTable, 80);
    decrypt_serial(t);
    encrypt_serial(version);
    memcpy(buffer + 0x100, (void*) &info[0], 80); //copy the info array to buffer
    memcpypgm2ram(buffer, xorTableptr, 0x100); //copy xortable to buffer
    FlashErase(addr); //erase data block
    WriteBuffer(addr); //write data block
}

//Rewrite Info

void WriteInfo(unsigned char *info_ptr) {
    unsigned char t;
    unsigned long addr = 0x1FC00;
    memcpypgm2ram(&t, (const far rom void*) DEVICE_TYPE_LOCATION, 1);
    memcpypgm2ram(buffer, (const far rom void*) addr, 0x400); //copy 1024bytes from flash to buffer
    memcpy(&info[0], (void*) info_ptr, 32); //copy device and serial code received over the usb to info array
    encrypt_serial(t); //encrypt the info array with (t) version of the key
    memcpy(buffer + 0x100, (void*) &info[0], 80); //copy the info array to buffer
    FlashErase(addr); //erase data block
    WriteBuffer(addr); //write data block
}

/* Test routine */
void load_shiftregister(unsigned char value) {
    unsigned char i;
    for (i = 0; i <= 7; i++) {
        SR_DAT = (value & 0x80) ? 1 : 0;
        value <<= 1;
        SR_CLK = 1;
        Delay10TCYx(2);
        SR_CLK = 0;
        Delay10TCYx(2);
    }
}

void config_io() {
    OSCTUNEbits.PLLEN = 1; //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
    Delay10KTCYx(20);
    WDTCONbits.ADSHR = 1; // Select alternate SFR location to access ANCONx registers
    ANCON0 = 0xFF; // Default all pins to digital
    ANCON1 = 0xFF; // Default all pins to digital
    WDTCONbits.ADSHR = 0; // Select normal SFR locations
    USBOutHandle = 0;
    USBInHandle = 0;
    USBDeviceInit();
    LATA = INIT_LATA;
    LATB = INIT_LATB;
    LATC = INIT_LATC;
    LATD = INIT_LATD;
    LATE = INIT_LATE;
    LATF = INIT_LATF;
    LATG = INIT_LATG;
    LATH = INIT_LATH;
    LATJ = INIT_LATJ;

    TRISA = INIT_TRISA;
    TRISB = INIT_TRISB;
    TRISC = INIT_TRISC;
    TRISD = INIT_TRISD;
    TRISE = INIT_TRISE;
    TRISF = INIT_TRISF;
    TRISG = INIT_TRISG;
    TRISH = INIT_TRISH;
    TRISJ = INIT_TRISJ;

    //Init IO expander
    load_shiftregister(0);
    LE0 = 1; //vpp0
    LE1 = 1; //vpp1
    LE5 = 1; //gnd0
    LE6 = 1; //gnd1
    LE7 = 1; //gnd2
    Delay10TCYx(2);
    LE0 = 0; //vpp0
    LE1 = 0; //vpp1
    LE5 = 0; //gnd0
    LE6 = 0; //gnd1
    LE7 = 0; //gnd2
    Delay10TCYx(2);
    load_shiftregister(0xFF);
    LE2 = 1; //vcc0
    LE3 = 1; //vcc1
    LE4 = 1; //vcc2
    Delay10TCYx(2);
    LE2 = 0; //vcc0
    LE3 = 0; //vcc1
    LE4 = 0; //vcc2
    Delay10TCYx(2);
    OE_VPP = 1;
    OE_VCC = 1;
    LED_PIN = 0;
}

void ReadProgMem(void) {
    memcpypgm2ram(&txdatapacket[0], (const far rom void*) rxdatapacket.pAdr, rxdatapacket.len);
}


// Process USB commands

void ProcessIo(void) {

    static unsigned char count = 0;
    if (USBSuspendControl == 1) {
        return;
    }
    if (USBDeviceState < CONFIGURED_STATE) {
        if (--led_cnt == 0) {
            LED_PIN ^= 1;
        }
        return;
    }
    if (led_cnt != 0) {
        LED_PIN = 1;
        led_cnt--;
    } else
        LED_PIN = 0;

    if (count) {
        if (!USBHandleBusy(USBInHandle)) {
            USBInHandle = USBGenWrite(USBGEN_EP_NUM, (BYTE*) & txdatapacket, count);
            count = 0;
        }
        return;
    }

    if (!USBHandleBusy(USBOutHandle)) {
        led_cnt = 5000U;
        count = 0;
        switch (rxdatapacket.CMD) {

            case RESET: // reset device
                reset_switch = 0x55AA55AA;
                DeviceReset();
                break;

            case READ_VERSION://get device state
                txdatapacket[0] = 0; //echo
                txdatapacket[1] = 1; //normal mode
                txdatapacket[2] = 33; //Length of devcode+serial+firmware version (LSB)
                txdatapacket[3] = 0; //Length of devcode+serial+firmware version (MSB)
                txdatapacket[4] = 0; //minor version
                txdatapacket[5] = 0; //major version
                txdatapacket[39] = 1; //Hardware version
                memcpypgm2ram(&txdatapacket[6], (const far rom void*) DEVICE_TYPE_LOCATION, 1);
                memcpypgm2ram(&txdatapacket[7], (const far rom void*) devcode, 8);
                memcpypgm2ram(&txdatapacket[15], (const far rom void*) serial, 24);
                count = 40;
                break;
            case READ_FLASH:
                ReadProgMem();
                count = rxdatapacket.len;
                break;

            case WRITE_BOOTLOADER:
                WriteBootloader(rxdatapacket._byte[1]);
                txdatapacket[0] = WRITE_BOOTLOADER;
                count = 1;
                break;

            case WRITE_CONFIG:
                memcpypgm2ram(buffer, (const far rom void*) 0x1FC00, 0x400); //copy 1024bytes from flash in buffer
                memcpypgm2ram(&buffer[0x3F8], (const far rom void*) config_bytes, 8); //Overwrite config bytes with default one
                if (!rxdatapacket._byte[1])//check for copy protect bit
                {
                    buffer[0x3F9] |= 0x4; //Clear copy protect bit
                }
                FlashErase(0x1FC00); //erase data block
                WriteBuffer(0x1FC00); //write data block
                txdatapacket[0] = WRITE_CONFIG;
                count = 1;
                break;

            case WRITE_INFO:
                WriteInfo(&rxdatapacket._byte[1]);
                //DeviceReset();
                txdatapacket[0] = WRITE_INFO;
                count = 1;
                break;

            case GET_INFO:
                memcpypgm2ram(&txdatapacket[32], (const far rom void*) DEVICE_TYPE_LOCATION, 1); //get the bootloader version
                memcpypgm2ram(&txdatapacket[33], (const far rom void*) DEVICE_COPY_PROTECT, 1); //get the copy protect byte
                memcpypgm2ram(&info[0], infoTable, 80); //copy the encrypted info array
                decrypt_serial(txdatapacket[32]); //decrypt the info array
                memcpy(txdatapacket, (void*) info, 32);
                txdatapacket[33] &= 0x4;
                count = 34;
                break;

            case GET_PORT_INP: // Get Port INP
                count = 1;
                switch (rxdatapacket._byte[1]) {
                    case 0:
                        txdatapacket[0] = PORTA;
                        break;
                    case 1:
                        txdatapacket[0] = PORTB;
                        break;
                    case 2:
                        txdatapacket[0] = PORTC;
                        break;
                    case 3:
                        txdatapacket[0] = PORTD;
                        break;
                    case 4:
                        txdatapacket[0] = PORTE;
                        break;
                    case 5:
                        txdatapacket[0] = PORTF;
                        break;
                    case 6:
                        txdatapacket[0] = PORTG;
                        break;
                    case 7:
                        txdatapacket[0] = PORTH;
                        break;
                    case 8:
                        txdatapacket[0] = PORTJ;
                        break;
                    default:
                        count = 0;
                        break;
                }
                break;

            case GET_PORT_LAT: // Get Port LAT
                count = 1;
                switch (rxdatapacket._byte[1]) {
                    case 0:
                        txdatapacket[0] = LATA;
                        break;
                    case 1:
                        txdatapacket[0] = LATB;
                        break;
                    case 2:
                        txdatapacket[0] = LATC;
                        break;
                    case 3:
                        txdatapacket[0] = LATD;
                        break;
                    case 4:
                        txdatapacket[0] = LATE;
                        break;
                    case 5:
                        txdatapacket[0] = LATF;
                        break;
                    case 6:
                        txdatapacket[0] = LATG;
                        break;
                    case 7:
                        txdatapacket[0] = LATH;
                        break;
                    case 8:
                        txdatapacket[0] = LATJ;
                        break;
                    default:
                        count = 0;
                        break;
                }
                break;



            case GET_PORT_TRIS: // Get Port TRIS
                count = 1;
                switch (rxdatapacket._byte[1]) {
                    case 0:
                        txdatapacket[0] = TRISA;
                        break;
                    case 1:
                        txdatapacket[0] = TRISB;
                        break;
                    case 2:
                        txdatapacket[0] = TRISC;
                        break;
                    case 3:
                        txdatapacket[0] = TRISD;
                        break;
                    case 4:
                        txdatapacket[0] = TRISE;
                        break;
                    case 5:
                        txdatapacket[0] = TRISF;
                        break;
                    case 6:
                        txdatapacket[0] = TRISG;
                        break;
                    case 7:
                        txdatapacket[0] = TRISH;
                        break;
                    case 8:
                        txdatapacket[0] = TRISJ;
                        break;
                    default:
                        count = 0;
                        break;
                }

                break;

            case SET_PORT_TRIS://Set ports Tris value
                switch (rxdatapacket._byte[1]) {
                    case 0:
                        if (rxdatapacket._byte[3])
                            TRISA |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISA &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 1:
                        if (rxdatapacket._byte[3])
                            TRISB |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISB &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 2:
                        if (rxdatapacket._byte[3])
                            TRISC |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISC &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 3:
                        if (rxdatapacket._byte[3])
                            TRISD |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISD &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 4:
                        if (rxdatapacket._byte[3])
                            TRISE |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISE &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 5:
                        if (rxdatapacket._byte[3])
                            TRISF |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISF &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 6:
                        if (rxdatapacket._byte[3])
                            TRISG |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISG &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 7:
                        if (rxdatapacket._byte[3])
                            TRISH |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISH &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 8:
                        if (rxdatapacket._byte[3])
                            TRISJ |= (1U << rxdatapacket._byte[2]);
                        else
                            TRISJ &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    default:
                        break;
                }
                break;


            case SET_PORT_LAT://Set ports Lat value
                switch (rxdatapacket._byte[1]) {
                    case 0:
                        if (rxdatapacket._byte[3])
                            LATA |= (1U << rxdatapacket._byte[2]);
                        else
                            LATA &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 1:
                        if (rxdatapacket._byte[3])
                            LATB |= (1U << rxdatapacket._byte[2]);
                        else
                            LATB &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 2:
                        if (rxdatapacket._byte[3])
                            LATC |= (1U << rxdatapacket._byte[2]);
                        else
                            LATC &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 3:
                        if (rxdatapacket._byte[3])
                            LATD |= (1U << rxdatapacket._byte[2]);
                        else
                            LATD &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 4:
                        if (rxdatapacket._byte[3])
                            LATE |= (1U << rxdatapacket._byte[2]);
                        else
                            LATE &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 5:
                        if (rxdatapacket._byte[3])
                            LATF |= (1U << rxdatapacket._byte[2]);
                        else
                            LATF &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 6:
                        if (rxdatapacket._byte[3])
                            LATG |= (1U << rxdatapacket._byte[2]);
                        else
                            LATG &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 7:
                        if (rxdatapacket._byte[3])
                            LATH |= (1U << rxdatapacket._byte[2]);
                        else
                            LATH &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    case 8:
                        if (rxdatapacket._byte[3])
                            LATJ |= (1U << rxdatapacket._byte[2]);
                        else
                            LATJ &= ~(1U << rxdatapacket._byte[2]);
                        break;
                    default:
                        break;
                }
                break;
            case SET_SHIFTREG:
                load_shiftregister(rxdatapacket._byte[1]);
                break;

            default: // Unknown command received
                break;
        }

        //rearm Out Endpoint
        USBOutHandle = USBGenRead(USBGEN_EP_NUM, (BYTE*) & rxdatapacket, USBGEN_EP_SIZE);
    }
}

// USB callback function handler

BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size) {
    switch (event) {
        case EVENT_CONFIGURED:
            //Enable the application endpoints
            USBEnableEndpoint(USBGEN_EP_NUM, USB_OUT_ENABLED | USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
            USBOutHandle = USBGenRead(USBGEN_EP_NUM, (BYTE*) & rxdatapacket, USBGEN_EP_SIZE);
            break;
        default:
            break;
    }
    return TRUE;
}

void main() {
    USBOutHandle = 0;
    USBInHandle = 0;
    led_cnt = 1;
    config_io();
    srand(0); //initialize the random number generator
#if defined (USB_INTERRUPT)
    USBDeviceAttach();
#endif
    while (1) {
#if defined(USB_POLLING)
        USBDeviceTasks();
#endif
        ProcessIo();

    }
}
