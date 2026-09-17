/* CAN transmit demo for the PIC18F25K80 (ECAN, Legacy mode).
 *
 * The on-chip ECAN module drives CANTX = RC6 (Arduino pin 22) and
 * CANRX = RC7 (Arduino pin 23).  Those are the same pins as EUSART1,
 * so a serial console cannot be used while CAN is active.
 *
 * A CAN transceiver (MCP2551, SN65HVD230, ...) is required between the
 * PIC and the bus; terminate the bus with 120 ohm at both ends.
 *
 * Every 500 ms a standard frame (ID 0x123, 3 data bytes) with a rolling
 * counter is sent, and every 8th iteration an extended frame as well.
 * The on-board LED toggles as a heartbeat.
 */

#include <Arduino.h>

#define FRAME_ID_STD 0x123
#define FRAME_ID_EXT 0x18FF50E5UL

void setup(void)
{
    pinMode(LED_BUILTIN, OUTPUT);

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
    static uint8_t counter = 0;
    uint8_t data[8];

    data[0] = counter++;
    data[1] = 0xAA;
    data[2] = 0x55;

    can_send_std(FRAME_ID_STD, data, 3);

    if ((counter & 0x07) == 0)
        can_send_ext(FRAME_ID_EXT, data, 8);

    digitalWrite(LED_BUILTIN, (counter & 0x01) ? HIGH : LOW);
    delay(500);
}
