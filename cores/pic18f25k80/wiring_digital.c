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
    case 8:  return 10;  /* RB0 / AN10 */
    case 9:  return 8;   /* RB1 / AN8  */
    case 12: return 9;   /* RB4 / AN9  */
    default: return 0xFF;
    }
}

static void clear_analog(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch <= 4)
        ANCON0 &= ~(1U << ch);
    else if (ch <= 10)
        ANCON1 &= ~(1U << (ch - 8));
}

static void set_analog(uint8_t pin)
{
    uint8_t ch = analog_channel_for_pin(pin);
    if (ch <= 4)
        ANCON0 |= (1U << ch);
    else if (ch <= 10)
        ANCON1 |= (1U << (ch - 8));
}

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    clear_analog(pin);
    if (mode == OUTPUT) {
        switch (port) {
        case 0: TRISA &= ~(1U << bit); LATA &= ~(1U << bit); break;
        case 1: TRISB &= ~(1U << bit); LATB &= ~(1U << bit); break;
        case 2: TRISC &= ~(1U << bit); LATC &= ~(1U << bit); break;
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
        }
    }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    if (val)
        switch (port) {
        case 0: LATA |= (1U << bit); break;
        case 1: LATB |= (1U << bit); break;
        case 2: LATC |= (1U << bit); break;
        }
    else
        switch (port) {
        case 0: LATA &= ~(1U << bit); break;
        case 1: LATB &= ~(1U << bit); break;
        case 2: LATC &= ~(1U << bit); break;
        }
}

int digitalRead(uint8_t pin)
{
    if (pin >= NUM_DIGITAL_PINS)
        return LOW;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    uint8_t v;
    switch (port) {
    case 0: v = (PORTA >> bit) & 1U; break;
    case 1: v = (PORTB >> bit) & 1U; break;
    case 2: v = (PORTC >> bit) & 1U; break;
    default: v = 0;
    }
    return v ? HIGH : LOW;
}