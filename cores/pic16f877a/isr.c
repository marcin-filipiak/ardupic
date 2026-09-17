#include <Arduino.h>

/* single interrupt vector (address 0x0004);
 * dispatches Timer1 tick and the INT0 callback.
 */

#define TMR1_PRELOAD_H 0xFC
#define TMR1_PRELOAD_L 0x18

volatile unsigned long _millis;

extern void (*_ext_int0)(void);

void _isr(void) __interrupt(0)
{
    if (PIR1 & _TMR1IF) {
        PIR1 &= ~_TMR1IF;
        TMR1H = TMR1_PRELOAD_H;
        TMR1L = TMR1_PRELOAD_L;
        _millis++;
    }
    if (INTCON & _INTF) {
        INTCON &= ~_INTF;
        if (_ext_int0)
            _ext_int0();
    }
}

unsigned long millis(void)
{
    unsigned long v;
    INTCON &= ~_GIE;
    v = _millis;
    INTCON |= _GIE;
    return v;
}

unsigned long micros(void)
{
    unsigned char l, h;
    unsigned int t;
    unsigned long m, us;

    INTCON &= ~_GIE;
    l = TMR1L;                 /* read low byte first to latch high */
    h = TMR1H;
    m = _millis;
    INTCON |= _GIE;

    t = l | ((unsigned int)h << 8);
    if (t < 0xFC18U) {
        if (m != 0UL)
            m--;
        t = 0xFFFFU - 0xFC18U + t + 1U;   /* counts since the pending rollover */
    } else {
        t = t - 0xFC18U;
    }
    us = (unsigned long)t;                /* 1 us per count @ 4 MHz */
    return m * 1000UL + us;
}