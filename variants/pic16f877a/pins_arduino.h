#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <Arduino.h>

/*
 * PIC16F877A (40-pin DIP)  -  Arduino numbering
 * pin = port*8 + bit  (port 0=PORTA, 1=PORTB, 2=PORTC, 3=PORTD, 4=PORTE)
 *
 *  digital 0..5  = RA0..RA5      analog A0..A4 (0..3, 5)
 *  digital 8     = RB0 / INT0
 *  digital 13    = LED_BUILTIN
 *  digital 18    = RC2 / CCP1 (PWM)
 *  digital 22    = RC6 / TX
 *  digital 23    = RC7 / RX
 *  digital 32..34= RE0..RE2 / AN5..AN7 -> A5..A7
 *  (RA6/RA7 are the crystal pins OSC1/OSC2 - unusable)
 */

#define A0  0
#define A1  1
#define A2  2
#define A3  3
#define A4  5
#define A5  32
#define A6  33
#define A7  34

#define PIN_PWM_P1A 18  /* RC2 / CCP1 */
#define PIN_TX 22
#define PIN_RX 23

#endif