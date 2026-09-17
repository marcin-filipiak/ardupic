#include <Arduino.h>

/* single high-priority ISR (sdcc allows only one vector per priority);
 * dispatches Timer0 tick and the three external interrupt callbacks.
 */

#define TMR0_PRELOAD_H 0xF0
#define TMR0_PRELOAD_L 0x60

volatile unsigned long _millis;

extern void (*_ext_int0)(void);
extern void (*_ext_int1)(void);
extern void (*_ext_int2)(void);

void _isr(void) __interrupt(1)
{
    if (INTCON & (1U << INTCON_TMR0IF)) {
        INTCON &= ~(1U << INTCON_TMR0IF);
        TMR0H = TMR0_PRELOAD_H;
        TMR0L = TMR0_PRELOAD_L;
        _millis++;
    }
    if (INTCON & (1U << INTCON_INT0IF)) {
        INTCON &= ~(1U << INTCON_INT0IF);
        if (_ext_int0)
            _ext_int0();
    }
    if (INTCON3 & (1U << INTCON3_INT1IF)) {
        INTCON3 &= ~(1U << INTCON3_INT1IF);
        if (_ext_int1)
            _ext_int1();
    }
    if (INTCON3 & (1U << INTCON3_INT2IF)) {
        INTCON3 &= ~(1U << INTCON3_INT2IF);
        if (_ext_int2)
            _ext_int2();
    }
}

unsigned long millis(void)
{
    unsigned long v;
    INTCON &= ~(1U << INTCON_GIE);
    v = _millis;
    INTCON |= (1U << INTCON_GIE);
    return v;
}

unsigned long micros(void)
{
    unsigned char l, h;
    unsigned int t;
    unsigned long m, us;

    INTCON &= ~(1U << INTCON_GIE);
    l = TMR0L;             /* TMR0H latched on TMR0L read */
    h = TMR0H;
    m = _millis;
    INTCON |= (1U << INTCON_GIE);

    t = l | ((unsigned int)h << 8);
    if (t < 0xF060U) {
        if (m != 0UL)
            m--;
        t = 0xFFFFU - 0xF060U + t + 1U;   /* counts since the pending rollover */
    } else {
        t = t - 0xF060U;
    }
    us = (unsigned long)t * 250UL / 1000UL;   /* 250 ns per count */
    return m * 1000UL + us;
}