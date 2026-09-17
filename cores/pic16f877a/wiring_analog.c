#include <Arduino.h>

static uint8_t analog_channel_for_pin(uint8_t pin)
{
    switch (pin) {
    case 0:  return 0;   /* RA0 / AN0 */
    case 1:  return 1;   /* RA1 / AN1 */
    case 2:  return 2;   /* RA2 / AN2 */
    case 3:  return 3;   /* RA3 / AN3 */
    case 5:  return 4;   /* RA5 / AN4 */
    case 32: return 5;   /* RE0 / AN5 */
    case 33: return 6;   /* RE1 / AN6 */
    case 34: return 7;   /* RE2 / AN7 */
    default: return 0xFF;
    }
}

/* PIC16F877A has a 10-bit ADC, 8 channels.
 * ADCS=010 -> TAD = Fosc/32 = 2 us @ 4 MHz (min 1.1 us ok).
 * Vref+ = VDD, Vref- = VSS. Returns 0 .. 1023.
 * PCFG: 0000 = all analog, 0111 = all digital.                    */

int analogRead(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch == 0xFF)
        return 0;

    pinMode(pin, INPUT);

    /* ADCON1: ADFM=1 (right justified), ADCS2=0, PCFG=0000 (all analog) */
    ADCON1 = _ADFM;

    /* ADCON0: ADON, ADCS=010 (ADCS0), CHS = channel */
    ADCON0 = _ADON | _ADCS0 | ((uint8_t)((ch & 7U) << 3));

    /* acquisition delay (~20 us for safety) */
    delayMicroseconds(20);

    /* start conversion */
    ADCON0 |= _GO_DONE;

    /* wait for completion */
    while (ADCON0 & _GO_DONE)
        ;

    /* 10-bit result, right justified: read ADRESH first (latch!) */
    unsigned int r = ((unsigned int)ADRESH << 2) |
                     ((unsigned int)(ADRESL & 0xC0U) >> 6);

    /* restore pins as digital (PCFG=0111, ADFM=1) */
    ADCON1 = _ADFM | _PCFG0 | _PCFG1 | _PCFG2;
    return (int)r;
}

/* analogWrite uses CCP1 PWM on pin 18 (RC2).
 * CCPR1L:CCP1Y/X = duty 10-bit; mode 0x0C = PWM (CCP1M=1100).
 * Frequency = Fosc/(4*(PR2+1)) with postscaler 1:1.              */

void analogWrite(uint8_t pin, int value)
{
    if (pin != 18)
        return;               /* CCP1 fixed to pin 18 (RC2) */

    if (value < 0)
        value = 0;
    if (value > 255)
        value = 255;

    TRISC &= ~(1U << 2);      /* RC2 = CCP1/P1A output */
    PR2 = 0xFF;               /* period = 256 * TMR2CLK (Fosc/4) */

    CCPR1L = (uint8_t)value;
    CCP1CON = (uint8_t)(0x0C | ((uint8_t)(value & 0x03) << 4));  /* PWM + CCP1Y/CCP1X */

    T2CON |= _TMR2ON;         /* start/keep Timer2 -> PWM runs */
}