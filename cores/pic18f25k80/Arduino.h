#ifndef Arduino_h
#define Arduino_h

#include <pic18f25k80.h>
#include <pic18f25k80_bits.h>
#include <stdint.h>

#define ARDUINO 10607
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define LSBFIRST 0
#define MSBFIRST 1

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define RISING 1
#define FALLING 2
#define CHANGE 3

#define DEFAULT 1
#define INTERNAL 2
#define EXTERNAL 0

#define CONSTRAIN_OK (0)
#define CONSTRAIN_MIN (1)
#define CONSTRAIN_MAX (2)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define bit(b) (1UL << (b))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define LED_BUILTIN 13

#define PIN_SERIAL_TX 22      /* RC6 */
#define PIN_SERIAL_RX 21      /* RC7 */

#define NUM_DIGITAL_PINS 23   /* PA0..7, PB0..7, PC0..6 */
#define NUM_ANALOG_INPUTS 8

/* ---- pin numberi ng: pin = port*8 + bit, port 0=A,1=B,2=C ---- */

void init(void);

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);

int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int value);

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

void attachInterrupt(uint8_t pin, void (*func)(void), uint8_t mode);
void detachInterrupt(uint8_t pin);

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

/* ---- UART1 (RC6/RX? RC6=TX, RC7=RX) ---- */
void uart1_begin(unsigned long baud);
void uart1_write(unsigned char c);
void uart1_print(const char *s);
void uart1_println(const char *s);
void uart1_print_ulong(unsigned long v);
void uart1_print_int(long v);
int uart1_read(void);
unsigned char uart1_available(void);

#include "pins_arduino.h"

#endif