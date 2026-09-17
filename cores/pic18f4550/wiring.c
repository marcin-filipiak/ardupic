#include <Arduino.h>

/* Timer0 in 16-bit mode, no prescaler.
 * T0CLK = Fosc/4; rollover count per 1 ms = Fosc/4000.
 *   48 MHz -> 12 MHz T0CLK -> 12000 counts (preload 0x10000-12000 = 0xD120)
 *    8 MHz ->  2 MHz T0CLK ->  2000 counts (preload 0x10000-2000  = 0xF830)
 */
static void tmr0_set_1ms_preload(void)
{
    unsigned int pre = (unsigned int)(0x10000U - (unsigned int)(F_CPU / 4000UL));
    TMR0H = (unsigned char)(pre >> 8);
    TMR0L = (unsigned char)(pre & 0xFF);
}

void init(void)
{
    /* Oscylator konfiguruje ustawienie bitu konfiguracyjnego FOSC
     * (HS-PLL na plytkach USB). NIE przestawiamy OSCCON, bo przelaczyl
     * by CPU na wewnetrzny oscylator. */

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
    tmr0_set_1ms_preload();
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

/* busy-counter delay; calibrated against Fosc (a loop iteration is
 * roughly 5 instruction cycles, T0I = Fosc/4 cycles per us).        */
void delayMicroseconds(unsigned int us)
{
    unsigned long i = ((unsigned long)us * (F_CPU / 4000000UL)) / 5UL;
    while (i-- != 0)
        ;
}