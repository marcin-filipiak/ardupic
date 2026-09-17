/* CAN receive demo for the PIC18F25K80 (ECAN, Legacy mode).
 *
 * CANTX = RC6 (Arduino pin 22), CANRX = RC7 (Arduino pin 23) - shared
 * with EUSART1, so no serial console is available while CAN is active.
 *
 * Incoming frames are queued by the RX interrupt (can_irq() from the
 * core ISR) and drained here with can_read().  Feedback:
 *   - the on-board LED toggles on every received frame;
 *   - data byte 0 of the frame is shown on digital pins 0..7
 *     (RA0..RA7) if LEDs are attached.
 *
 * A CAN transceiver (MCP2551, SN65HVD230, ...) is required between the
 * PIC and the bus; terminate the bus with 120 ohm at both ends.
 */

#include <Arduino.h>

static unsigned long frames;

static void show_byte(uint8_t v)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
        digitalWrite(i, (v >> i) & 1U);
}

void setup(void)
{
    uint8_t i;

    pinMode(LED_BUILTIN, OUTPUT);
    for (i = 0; i < 8; i++)
        pinMode(i, OUTPUT);

    if (!can_begin(CAN_SPEED_250)) {
        for (;;) {                  /* no valid bit timing: slow blink */
            digitalWrite(LED_BUILTIN, HIGH);
            delay(200);
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
        }
    }
}

void loop(void)
{
    can_message_t msg;

    if (can_read(&msg)) {
        frames++;
        digitalWrite(LED_BUILTIN, (frames & 1UL) ? HIGH : LOW);
        if (msg.length > 0)
            show_byte(msg.data[0]);
    }
}
