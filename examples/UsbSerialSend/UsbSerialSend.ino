/* USB CDC transmit demo (virtual serial) for PIC18F2550 / PIC18F4550.
 *
 * The boards run at 48 MHz (HS-PLL, 20 MHz crystal).  Open the port
 * (e.g. /dev/ttyACM0) at any baud rate - USB CDC ignores it.
 *
 * Every 300 ms a line with the uptime, the analog value on A0 and a
 * rolling counter is sent to the host.  usbcdc_poll() is called in the
 * loop so the USB engine can service control transfers.
 */

#include <Arduino.h>

void setup(void)
{
    usbcdc_begin();
    usbcdc_println("ardupic USB CDC send demo");
}

void loop(void)
{
    static unsigned long counter = 0;

    usbcdc_print("t=");
    usbcdc_print_ulong(millis());
    usbcdc_print(" A0=");
    usbcdc_print_int(analogRead(A0));
    usbcdc_print(" n=");
    usbcdc_print_ulong(counter++);
    usbcdc_println("");

    usbcdc_poll();
    delay(300);
}
