#include <Arduino.h>

static uint8_t analog_channel_for_pin(uint8_t pin)
{
    /* returns ADC channel number or 0xFF if pin has no analog function */
    switch (pin) {
    case 0:  return 0;   /* RA0 / AN0 */
    case 1:  return 1;   /* RA1 / AN1 */
    case 2:  return 2;   /* RA2 / AN2 */
    case 3:  return 3;   /* RA3 / AN3 */
    case 5:  return 4;   /* RA5 / AN4 */
    case 32: return 5;   /* RE0 / AN5 */
    case 33: return 6;   /* RE1 / AN6 */
    case 34: return 7;   /* RE2 / AN7 */
    case 8:  return 12;  /* RB0 / AN12 */
    case 9:  return 10;  /* RB1 / AN10 */
    case 10: return 8;   /* RB2 / AN8 */
    case 11: return 9;   /* RB3 / AN9 */
    case 12: return 11;  /* RB4 / AN11 */
    case 13: return 13;  /* RB5 / AN13 */
    default: return 0xFF;
    }
}

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    if (pin == 7)                /* RA7 nie istnieje w PIC18F4550 */
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;

    if (mode == OUTPUT) {
        switch (port) {
        case 0: TRISA &= ~(1U << bit); LATA &= ~(1U << bit); break;
        case 1: TRISB &= ~(1U << bit); LATB &= ~(1U << bit); break;
        case 2: TRISC &= ~(1U << bit); LATC &= ~(1U << bit); break;
        case 3: TRISD &= ~(1U << bit); LATD &= ~(1U << bit); break;
        case 4: TRISE &= ~(1U << bit); LATE &= ~(1U << bit); break;
        }
    } else {
        switch (port) {
        case 0: TRISA |= (1U << bit); break;
        case 1:
            TRISB |= (1U << bit);
            if (mode == INPUT_PULLUP)
                INTCON2 &= ~(1U << INTCON2_NOT_RBPU);   /* global PORTB pull-up enable */
            break;
        case 2: TRISC |= (1U << bit); break;
        case 3: TRISD |= (1U << bit); break;
        case 4: TRISE |= (1U << bit); break;
        }
    }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    if (pin == 7)
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    if (val)
        switch (port) {
        case 0: LATA |= (1U << bit); break;
        case 1: LATB |= (1U << bit); break;
        case 2: LATC |= (1U << bit); break;
        case 3: LATD |= (1U << bit); break;
        case 4: LATE |= (1U << bit); break;
        }
    else
        switch (port) {
        case 0: LATA &= ~(1U << bit); break;
        case 1: LATB &= ~(1U << bit); break;
        case 2: LATC &= ~(1U << bit); break;
        case 3: LATD &= ~(1U << bit); break;
        case 4: LATE &= ~(1U << bit); break;
        }
}

int digitalRead(uint8_t pin)
{
    if (pin >= NUM_DIGITAL_PINS)
        return LOW;
    if (pin == 7)
        return LOW;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    uint8_t v;
    switch (port) {
    case 0: v = (PORTA >> bit) & 1U; break;
    case 1: v = (PORTB >> bit) & 1U; break;
    case 2: v = (PORTC >> bit) & 1U; break;
    case 3: v = (PORTD >> bit) & 1U; break;
    case 4: v = (PORTE >> bit) & 1U; break;
    default: v = 0;
    }
    return v ? HIGH : LOW;
}