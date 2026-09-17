#include <Arduino.h>

/* PIC16F877A configuration - a single config word @ 0x2007.
 * 0x3FF1 : FOSC=XT, WDTE=off, PWRTE off, BOREN off, LVP off, CP off.
 * SDCC (pic14) does not support #pragma config - we use __at(0x2007). */
__code unsigned int __at(0x2007) __CONFIG = 0x3FF1;

extern void setup(void);
extern void loop(void);

void main(void)
{
    init();
    setup();
    for (;;)
        loop();
}