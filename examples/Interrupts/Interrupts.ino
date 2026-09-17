/* External interrupt counter (INT0 = RB0 = digital pin 8).
 *
 * Counts falling edges and reports the total over UART1 at 9600 baud
 * every 500 ms.  Works on every supported board (PIC16F877A exposes
 * only INT0; the PIC18 parts also allow pins 9 and 10).
 *
 * Drive pin 8 from a push-button (to GND) or a signal generator.  No
 * debouncing is done, so a mechanical button may count a few extra
 * edges - add an RC filter or a Schmitt trigger if that matters.
 */

#include <Arduino.h>

static volatile unsigned long pulses;

static void on_pulse(void)
{
    pulses++;
}

void setup(void)
{
    pinMode(8, INPUT_PULLUP);
    uart1_begin(9600);
    attachInterrupt(8, on_pulse, FALLING);
    uart1_println("ardupic interrupt counter");
}

void loop(void)
{
    unsigned long a, b;

    do {                        /* consistent snapshot of the 32-bit counter */
        a = pulses;
        b = pulses;
    } while (a != b);

    uart1_print("pulses=");
    uart1_print_ulong(a);
    uart1_println("");
    delay(500);
}
