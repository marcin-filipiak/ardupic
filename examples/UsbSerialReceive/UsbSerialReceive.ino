/* USB CDC receive demo (virtual serial) for PIC18F2550 / PIC18F4550.
 *
 * Reads bytes sent by the host, echoes each one back and interprets
 * simple single-letter commands.  Run with any serial terminal:
 *
 *   o      LED on                 f   LED off
 *   t      toggle LED             ?   report state
 *   0-9    PWM on pin 18 to 0..9 * 28
 *
 * Any other byte is counted and echoed as-is.  The boards run at 48 MHz
 * (HS-PLL, 20 MHz crystal); the baud rate is ignored.
 */

#include <Arduino.h>

static unsigned char led;
static unsigned long received;

static void command(int c)
{
    switch (c) {
    case 'o':
        led = 1;
        break;
    case 'f':
        led = 0;
        break;
    case 't':
        led = (unsigned char)!led;
        break;
    case '?':
        usbcdc_print("led=");
        usbcdc_print_int(led);
        usbcdc_print(" rx=");
        usbcdc_print_ulong(received);
        usbcdc_println("");
        break;
    default:
        if (c >= '0' && c <= '9')
            analogWrite(18, (c - '0') * 28);
        break;
    }
    digitalWrite(LED_BUILTIN, led ? HIGH : LOW);
}

void setup(void)
{
    usbcdc_begin();
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(18, OUTPUT);
    usbcdc_println("ardupic USB CDC receive demo - send o/f/t/?/0-9");
}

void loop(void)
{
    int c = usbcdc_read();

    if (c >= 0) {
        received++;
        usbcdc_write((unsigned char)c);      /* echo */
        command(c);
    }

    usbcdc_poll();
}
