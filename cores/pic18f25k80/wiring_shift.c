#include <Arduino.h>

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST)
            digitalWrite(dataPin, (uint8_t)((val >> i) & 1U));
        else
            digitalWrite(dataPin, (uint8_t)((val >> (7 - i)) & 1U));
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
    uint8_t value = 0, i;
    for (i = 0; i < 8; i++) {
        digitalWrite(clockPin, HIGH);
        if (bitOrder == LSBFIRST)
            value |= (digitalRead(dataPin) ? (uint8_t)(1U << i) : 0U);
        else
            value |= (digitalRead(dataPin) ? (uint8_t)(1U << (7 - i)) : 0U);
        digitalWrite(clockPin, LOW);
    }
    return value;
}