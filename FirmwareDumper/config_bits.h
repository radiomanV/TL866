/* 
 * File:   config_bits.h
 * Author: Radioman
 *
 * Created on 09 aprilie 2013, 07:13
 */

#ifndef CONFIG_BITS_H
#define	CONFIG_BITS_H

// PIC18F87J50 Configuration Bit Settings
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT disabled (control is placed on SWDTEN bit))
#pragma config PLLDIV = 4       // PLL Prescaler Selection bits (Divide by 4 (16 MHz oscillator input))
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Reset on stack overflow/underflow enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))
#pragma config DEBUG = OFF      // Debugging off

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide); 48Mhz CPU Clock
#pragma config CP0 = OFF        // Code Protection bit (Program memory is not code-protected)

// CONFIG2L
#pragma config FOSC = HSPLL     // Oscillator Selection bits (HS oscillator, PLL enabled, HSPLL used by USB)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit (Two-Speed Start-up enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Timer Postscaler Select bits (1:32768)

// CONFIG3L
#pragma config EASHFT = ON      // External Address Bus Shift Enable bit (Address shifting enabled, address on external bus is offset to start at 000000h)
#pragma config MODE = MM        // External Memory Bus Configuration bits (Microcontroller mode - External bus disabled)
#pragma config BW = 16          // Data Bus Width Select bit (16-bit external bus mode)
#pragma config WAIT = OFF       // External Bus Wait Enable bit (Wait states on the external bus are disabled)

// CONFIG3H
#pragma config CCP2MX = DEFAULT // ECCP2 MUX bit (ECCP2/P2A is multiplexed with RC1)
#pragma config ECCPMX = DEFAULT // ECCPx MUX bit (ECCP1 outputs (P1B/P1C) are multiplexed with RE6 and RE5; ECCP3 outputs (P3B/P3C) are multiplexed with RE4 and RE3)
#pragma config PMPMX = DEFAULT  // PMP Pin Multiplex bit (PMP pins placed on EMB)
#pragma config MSSPMSK = MSK7   // MSSP Address Masking Mode Select bit (7-Bit Address Masking mode enable)

#endif	/* CONFIG_BITS_H */

