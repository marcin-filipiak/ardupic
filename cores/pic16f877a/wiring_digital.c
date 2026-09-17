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
    default: return 0xFF;
    }
}

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    if (pin == 6 || pin == 7)        /* RA6/RA7 = piny krysztalu (OSC2/OSC1) */
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;

    if (mode == OUTPUT) {
        /* najpierw wyjcie, potem kierunek - uniknij stanu wysokiej Z z impulsem */
        switch (port) {
        case 0: PORTA &= ~(1U << bit); TRISA &= ~(1U << bit); break;
        case 1: PORTB &= ~(1U << bit); TRISB &= ~(1U << bit); break;
        case 2: PORTC &= ~(1U << bit); TRISC &= ~(1U << bit); break;
        case 3: PORTD &= ~(1U << bit); TRISD &= ~(1U << bit); break;
        case 4: PORTE &= ~(1U << bit); TRISE &= ~(1U << bit); break;
        }
    } else {
        switch (port) {
        case 0: TRISA |= (1U << bit); break;
        case 1:
            TRISB |= (1U << bit);
            if (mode == INPUT_PULLUP) {
                OPTION_REG &= ~0x80U;            /* PORTB pull-up enable */
                PORTB |= (1U << bit);            /* pull-up na tym pinie */
            }
            break;
        case 2: TRISC |= (1U << bit); break;
        case 3: TRISD |= (1U << bit); break;
        case 4: TRISE |= (1U << bit); break;
        }
    }
}

/* PIC16F877A nie ma rejestrow LAT - piszemy bezposrednio do PORT.
 * UWAGA: odczyt-zapis (RMW) na pinach PORTB moze zaklocic ustawienie
 * innych pinow wejsciowych tego portu (klasyczny problem PIC16). */
void digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin >= NUM_DIGITAL_PINS)
        return;
    if (pin == 6 || pin == 7)
        return;
    uint8_t port = pin >> 3;
    uint8_t bit  = pin & 7;
    if (val)
        switch (port) {
        case 0: PORTA |= (1U << bit); break;
        case 1: PORTB |= (1U << bit); break;
        case 2: PORTC |= (1U << bit); break;
        case 3: PORTD |= (1U << bit); break;
        case 4: PORTE |= (1U << bit); break;
        }
    else
        switch (port) {
        case 0: PORTA &= ~(1U << bit); break;
        case 1: PORTB &= ~(1U << bit); break;
        case 2: PORTC &= ~(1U << bit); break;
        case 3: PORTD &= ~(1U << bit); break;
        case 4: PORTE &= ~(1U << bit); break;
        }
}

int digitalRead(uint8_t pin)
{
    if (pin >= NUM_DIGITAL_PINS)
        return LOW;
    if (pin == 6 || pin == 7)
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