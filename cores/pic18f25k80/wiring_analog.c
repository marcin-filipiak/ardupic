#include <Arduino.h>

static uint8_t analog_channel_for_pin(uint8_t pin)
{
    switch (pin) {
    case 0:  return 0;
    case 1:  return 1;
    case 2:  return 2;
    case 3:  return 3;
    case 5:  return 4;
    case 8:  return 10;
    case 9:  return 8;
    case 12: return 9;
    default: return 0xFF;
    }
}

static void set_analog(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch <= 4)
        ANCON0 |= (1U << ch);
    else if (ch <= 10)
        ANCON1 |= (1U << (ch - 8));
}

/* K80 has 12-bit ADC. Right-justified (ADFM=1).
 * Vref+ = VDD, Vref- = VSS. Acquisition time ~12 TAD after GO.
 * Returns 0 .. 4095.                                              */

int analogRead(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch == 0xFF)
        return 0;

    set_analog(pin);
    pinMode(pin, INPUT);

    /* ADCON1: VCFG1=0 VDD ref+, VCFG0=0 VSS ref-, CHSN=0 */
    ADCON1 = 0x00;

    /* ADCON2: ADFM=1 (12-bit right justified), ADCS=101 (Fosc/64 -> TAD=4us) */
    ADCON2 = (1U << ADCON2_ADFM) | (5U);

    /* select channel & enable ADC */
    ADCON0 = ((uint8_t)(ch << 2)) | (1U << ADCON0_ADON);

    /* acquisition delay (minimum 12 µs @ 5 V, ~20 µs for safety) */
    delayMicroseconds(20);

    /* start conversion */
    ADCON0 |= (1U << ADCON0_GO);

    /* wait for completion */
    while (ADCON0 & (1U << ADCON0_GO))
        ;

    /* 12-bit result: ADRESH(7..0)=bits(11..4), ADRESL(7..4)=bits(3..0) */
    return (int)(((unsigned int)ADRESH << 4) | ((unsigned int)(ADRESL & 0x0F)));
}

/* analogWrite uses CCP1 PWM on pin 18 (RC2).
 * CCP1 = ECCP1 with P1A..P1D on RC2/RB1/RB2/RB3.
 * We only use P1A (RC2). Frequency = Fosc/(4*PR2) with postscaler 1:1.
 * duty = CCPR1L:DC1B(1:0) in T2CON bits. 0%..100%.                   */

void analogWrite(uint8_t pin, int value)
{
    if (pin != 18)
        return;               /* CCP1 fixed to pin 18 (RC2) */

    if (value < 0)
        value = 0;
    if (value > 255)
        value = 255;

    TRISC &= ~(1U << 2);      /* RC2 = P1A output */
    PR2 = 0xFF;               /* period = 256 * TMR2CLK (Fosc/4) */

    CCPR1L = (uint8_t)value;
    ECCP1CON = (uint8_t)(0x0C | ((uint8_t)(value & 0x03) << 4));  /* PWM + DC1B1:0 */

    T2CON |= (1U << T2CON_TMR2ON);  /* start/keep Timer2 -> PWM runs */
}