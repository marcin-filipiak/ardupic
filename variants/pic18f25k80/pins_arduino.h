#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <Arduino.h>

/*
 * PIC18F25K80 (28-pin DIP)  -  Arduino numbering
 * pin = port*8 + bit  (port 0=PORTA, 1=PORTB, 2=PORTC)
 *
 *  digital 0..7  = RA0..RA7     analog A0..A3 (0..3), A4 (5)
 *  digital 8     = RB0 / INT0 / AN10  -> A5
 *  digital 9     = RB1 / INT1 / AN8   -> A6
 *  digital 12    = RB4 / AN9          -> A7
 *  digital 18    = RC2 / CCP1 (PWM)
 *  digital 21    = RC7 / RX
 *  digital 22    = RC6 / TX
 */

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 5
#define A5 8
#define A6 9
#define A7 12

#define PIN_PWM_P1A 18  /* RC2 / CCP1 */
#define PIN_TX 22
#define PIN_RX 21

#endif