/* Blink the on-board LED without delay(): the loop stays responsive.
 * Uses millis(), which is driven by the Timer0 interrupt on every board.
 */

#include <Arduino.h>

#define INTERVAL 500UL

static unsigned long previous;
static int state = LOW;

void setup(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void)
{
    unsigned long now = millis();

    if ((now - previous) >= INTERVAL) {
        previous = now;
        state = !state;
        digitalWrite(LED_BUILTIN, state);
    }
}
