#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <Arduino.h>

/*
 * PIC18F4550 (40-pin DIP)  -  Arduino numbering
 * pin = port*8 + bit  (port 0=PORTA, 1=PORTB, 2=PORTC, 3=PORTD, 4=PORTE)
 *
 *  digital 0..6  = RA0..RA6      analog A0..A3 (0..3), A4 (5)
 *  digital 8     = RB0 / INT0 / AN12 -> A12
 *  digital 9     = RB1 / INT1 / AN10 -> A10
 *  digital 10    = RB2 / INT2 / AN8  -> A8
 *  digital 11    = RB3 / AN9         -> A9
 *  digital 12    = RB4 / AN11        -> A11
 *  digital 13    = RB5 / AN13        -> A13
 *  digital 18    = RC2 / CCP1 (PWM)
 *  digital 22    = RC6 / TX
 *  digital 23    = RC7 / RX
 *  digital 32..34= RE0..RE2 / AN5..AN7 -> A5..A7
 *  (RA7 nie istnieje; RC4/RC5 = USB)
 *  (RA6/RA7 = OSC2/OSC1 - zajete przez krysztal HS-PLL w konfiguracji USB 48 MHz)
 */

#define A0  0
#define A1  1
#define A2  2
#define A3  3
#define A4  5
#define A5  32
#define A6  33
#define A7  34
#define A8  10
#define A9  11
#define A10 9
#define A11 12
#define A12 8
#define A13 13

#define PIN_PWM_P1A 18  /* RC2 / CCP1 */
#define PIN_TX 22
#define PIN_RX 23

#endif