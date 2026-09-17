#include <Arduino.h>

/* Timer1 in 16-bit mode, no prescaler.
 * Fosc = 4 MHz -> T1CLK = 1 MHz = 1 us / count.
 * Rollover period = 1000 counts = 1000 us.
 *
 * Preload: 0x10000 - 1000 = 0xFC18  (TMR1H=0xFC, TMR1L=0x18)
 */
#define TMR1_PRELOAD_H 0xFC
#define TMR1_PRELOAD_L 0x18

void init(void)
{
    /* ADCON1: ADFM=1, PCFG=0111 -> wszystkie kanaly cyfrowe */
    ADCON1 = _ADFM | _PCFG0 | _PCFG1 | _PCFG2;

    /* all pins inputs, output states low */
    TRISA = 0xFF; PORTA = 0x00;
    TRISB = 0xFF; PORTB = 0x00;
    TRISC = 0xFF; PORTC = 0x00;
    TRISD = 0xFF; PORTD = 0x00;
    TRISE = 0xFF; PORTE = 0x00;

    /* Timer1 16-bit, internal clock, prescaler 1:1, stopped */
    T1CON = 0x00;
    TMR1H = TMR1_PRELOAD_H;
    TMR1L = TMR1_PRELOAD_L;
    T1CON = _TMR1ON;

    /* run it: interrupt at every 1 ms */
    PIR1 &= ~_TMR1IF;
    PIE1 = _TMR1IE;
    INTCON = 0x00;
    INTCON |= _GIE | _PEIE;
}

void delay(unsigned long ms)
{
    unsigned long start = millis();
    while ((millis() - start) < ms)
        ;
}

/* busy-counter delay; calibrated for Fosc = 4 MHz (1 instr / us).
 * Each loop iteration is roughly 4-5 instructions -> factor 1/5.   */
void delayMicroseconds(unsigned int us)
{
    unsigned int i = us / 5;
    while (i != 0)
        i--;
}