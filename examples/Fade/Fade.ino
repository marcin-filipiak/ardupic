/* PWM fade on pin 18 (RC2 / CCP1).  Works on every supported board:
 * PIC18F25K80, PIC18F2550, PIC18F4550 and PIC16F877A.
 */

#include <Arduino.h>

void setup(void)
{
    pinMode(18, OUTPUT);
}

void loop(void)
{
    int v;

    for (v = 0; v <= 255; v += 5) {
        analogWrite(18, v);
        delay(10);
    }
    for (v = 255; v >= 0; v -= 5) {
        analogWrite(18, v);
        delay(10);
    }
}
