#include <Arduino.h>

/* Timer0 in 16-bit mode, no prescaler.
 * Fosc = 16 MHz -> T0CLK = 4 MHz = 250 ns / count.
 * Rollover period = 4000 counts = 1000 us.
 *
 * Preload: 0x10000 - 4000 = 0xF060  (TMR0H=0xF0, TMR0L=0x60)
 */
#define TMR0_PRELOAD_H 0xF0
#define TMR0_PRELOAD_L 0x60

void init(void)
{
    /* keep every pin digital until an analog read enables its channel */
    ANCON0 = 0x00;
    ANCON1 = 0x00;

    /* output latches low, all pins inputs */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;

    /* 16-bit Timer0, internal clock, no prescaler, stopped */
    T0CON = 0x00;
    TMR0H = TMR0_PRELOAD_H;
    TMR0L = TMR0_PRELOAD_L;
    T0CON = (1U << T0CON_TMR0ON);      /* 16-bit, PS=1 */

    /* run it: interrupt at every 1 ms */
    INTCON &= ~(1U << INTCON_TMR0IF);
    INTCON |= (1U << INTCON_TMR0IE) | (1U << INTCON_GIE);
}

void delay(unsigned long ms)
{
    unsigned long start = millis();
    while ((millis() - start) < ms)
        ;
}

/* busy-counter delay; calibrated for Fosc = 16 MHz (4 instr / us).
 * Each loop iteration is roughly 4-5 instructions -> factor 4/5.   */
void delayMicroseconds(unsigned int us)
{
    unsigned int i = (unsigned int)(((unsigned long)us * 4UL) / 5UL);
    while (i-- != 0)
        ;
}