#include <Arduino.h>

void (*_ext_int0)(void);
void (*_ext_int1)(void);
void (*_ext_int2)(void);

/* INT0 edge is configured once in INTCON2/INTEDG0.                *
 * For mode == CHANGE we re-arm the edge on each interrupt so the  *
 * opposite edge is requested next time (reads current pin state).  *
 * This gives an approximate CHANGE handler (at most one extra      *
 * transition delay).                                               */

void attachInterrupt(uint8_t pin, void (*func)(void), uint8_t mode)
{
    switch (pin) {
    case 8:   /* INT0 = RB0 = Arduino digital pin 8 */
        _ext_int0 = func;
        if (mode == RISING)
            INTCON2 |= (1U << INTCON2_INTEDG0);
        else
            INTCON2 &= ~(1U << INTCON2_INTEDG0);
        INTCON |= (1U << INTCON_INT0IF) | (1U << INTCON_INT0IE);
        break;
    case 9:   /* INT1 = RB1 */
        _ext_int1 = func;
        if (mode == RISING)
            INTCON2 |= (1U << INTCON2_INTEDG1);
        else
            INTCON2 &= ~(1U << INTCON2_INTEDG1);
        INTCON3 |= (1U << INTCON3_INT1IF) | (1U << INTCON3_INT1IE);
        break;
    case 10:  /* INT2 = RB2 */
        _ext_int2 = func;
        if (mode == RISING)
            INTCON2 |= (1U << INTCON2_INTEDG2);
        else
            INTCON2 &= ~(1U << INTCON2_INTEDG2);
        INTCON3 |= (1U << INTCON3_INT2IF) | (1U << INTCON3_INT2IE);
        break;
    default:
        break;
    }
}

void detachInterrupt(uint8_t pin)
{
    switch (pin) {
    case 8:
        INTCON &= ~(1U << INTCON_INT0IE);
        _ext_int0 = (void (*)(void))0;
        break;
    case 9:
        INTCON3 &= ~(1U << INTCON3_INT1IE);
        _ext_int1 = (void (*)(void))0;
        break;
    case 10:
        INTCON3 &= ~(1U << INTCON3_INT2IE);
        _ext_int2 = (void (*)(void))0;
        break;
    default:
        break;
    }
}