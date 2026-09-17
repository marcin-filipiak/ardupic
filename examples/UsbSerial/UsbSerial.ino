/* USB CDC (virtual serial) echo over the on-chip USB module.
 * PIC18F2550 / PIC18F4550 boards run at 48 MHz (HS-PLL).
 * Open the port (e.g. /dev/ttyACM0) at any baud rate; bytes are echoed back.
 */
void setup(void)
{
    usbcdc_begin();
    usbcdc_println("ardupic USB CDC ready");
}

void loop(void)
{
    int c = usbcdc_read();

    if (c >= 0) {
        usbcdc_write((unsigned char)c);
        if (c == '\r')
            usbcdc_write('\n');
    }

    usbcdc_poll();
}