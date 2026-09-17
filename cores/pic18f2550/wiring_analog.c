#include <Arduino.h>

static uint8_t analog_channel_for_pin(uint8_t pin)
{
    switch (pin) {
    case 0:  return 0;   /* RA0 / AN0 */
    case 1:  return 1;   /* RA1 / AN1 */
    case 2:  return 2;   /* RA2 / AN2 */
    case 3:  return 3;   /* RA3 / AN3 */
    case 5:  return 4;   /* RA5 / AN4 */
    case 8:  return 12;  /* RB0 / AN12 */
    case 9:  return 10;  /* RB1 / AN10 */
    case 10: return 8;   /* RB2 / AN8  */
    case 11: return 9;   /* RB3 / AN9  */
    case 12: return 11;  /* RB4 / AN11 */
    case 13: return 13;  /* RB5 / AN13 */
    default: return 0xFF;
    }
}

static uint8_t pcfg_for_channel(uint8_t ch)
{
    /* PCFG<3:0> for "keep digital, enable just this channel (range)".
     * Channels 0..6 enable that single pin; higher channels come as a
     * group covering AN0..ANch.  Datasheet DS39632E Table 19-1.       */
    static const uint8_t table[14] = {
        0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
        0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
    };
    if (ch > 13)
        return ADCON1_PCFG_ALL_DIG;
    return table[ch];
}

/* PIC18F2550 has 10-bit ADC. Right-justified (ADFM=1).
 * Vref+ = VDD, Vref- = VSS. Acquisition time ~12 TAD after GO.
 * Returns 0 .. 1023.                                              */

int analogRead(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch == 0xFF)
        return 0;

    pinMode(pin, INPUT);

    /* ADCON2: ADFM=1 (right justified), ADCS=101 (Fosc/64 -> TAD=4us) */
    ADCON2 = (1U << ADCON2_ADFM) | (5U);

    /* select channel & enable ADC */
    ADCON0 = ((uint8_t)(ch << 2)) | (1U << ADCON0_ADON);

    /* enable just this channel (the rest stay digital) */
    ADCON1 = pcfg_for_channel(ch);

    /* acquisition delay (minimum 12 µs @ 5 V, ~20 µs for safety) */
    delayMicroseconds(20);

    /* start conversion */
    ADCON0 |= (1U << ADCON0_GO);

    /* wait for completion */
    while (ADCON0 & (1U << ADCON0_GO))
        ;

    /* 10-bit result, right justified: (ADRESH<<2) | (ADRESL>>6) */
    int r = (int)(((unsigned int)ADRESH << 2) |
                  ((unsigned int)(ADRESL & 0xC0U) >> 6));

    /* restore all pins as digital */
    ADCON1 = ADCON1_PCFG_ALL_DIG;
    return r;
}

/* analogWrite uses CCP1 PWM on pin 18 (RC2).
 * CCPR1L:DC1B(1:0) = duty (PWM period = (PR2+1)*TMR2CLK, 0%..100%).
 * Frequency = Fosc/(4*(PR2+1)) with postscaler 1:1.                 */

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
    CCP1CON = (uint8_t)(0x0C | ((uint8_t)(value & 0x03) << 4));  /* PWM + DC1B1:0 */

    T2CON |= (1U << T2CON_TMR2ON);  /* start/keep Timer2 -> PWM runs */
}