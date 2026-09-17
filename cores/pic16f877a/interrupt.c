#include <Arduino.h>

void (*_ext_int0)(void);

/* INT0 edge is set from OPTION_REG/INTEDG.
 * Only INT0 (pin 8 = RB0) is exposed on PIC16F877A.
 */

void attachInterrupt(uint8_t pin, void (*func)(void), uint8_t mode)
{
    if (pin != 8)
        return;
    _ext_int0 = func;
    if (mode == RISING)
        OPTION_REG |= _INTEDG;
    else
        OPTION_REG &= ~_INTEDG;
    INTCON |= _INTF | _INTE;
}

void detachInterrupt(uint8_t pin)
{
    if (pin != 8)
        return;
    INTCON &= ~_INTE;
    _ext_int0 = (void (*)(void))0;
}