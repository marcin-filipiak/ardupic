#include <Arduino.h>

/* Timer0 in 16-bit mode, no prescaler.
 * Fosc = 8 MHz -> T0CLK = 2 MHz = 500 ns / count.
 * Rollover period = 2000 counts = 1000 us.
 *
 * Preload: 0x10000 - 2000 = 0xF830  (TMR0H=0xF8, TMR0L=0x30)
 */
#define TMR0_PRELOAD_H 0xF8
#define TMR0_PRELOAD_L 0x30

void init(void)
{
    /* wewnetrzny oscylator 8 MHz (IRCF=110), czekaj na stabilizacje */
    OSCCON = (1U << OSCCON_IRCF2) | (1U << OSCCON_IRCF1) | (1U << OSCCON_IDLEN);
    while (!(OSCCON & (1U << OSCCON_IOFS)))
        ;

    /* wszystkie kanaly cyfrowe dopoki analogRead nie wlaczy danego */
    ADCON1 = ADCON1_PCFG_ALL_DIG;

    /* output latches low, all pins inputs */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;
    TRISD = 0xFF;
    TRISE = 0xFF;

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

/* busy-counter delay; calibrated for Fosc = 8 MHz (2 instr / us).
 * Each loop iteration is roughly 4-5 instructions -> factor 2/5.   */
void delayMicroseconds(unsigned int us)
{
    unsigned int i = (unsigned int)(((unsigned long)us * 2UL) / 5UL);
    while (i-- != 0)
        ;
}