/* Read digital pin 8 and print the state over UART1 at 9600 baud.
 * TX = RC6 (pin 22), RX = RC7 (pin 23 on the 40-pin parts, pin 21 on
 * the PIC18F25K80).  Works on every supported board.
 *
 * Wire a button or switch between pin 8 (RB0) and GND; the internal
 * pull-up keeps the pin high when the button is released.
 */

#include <Arduino.h>

void setup(void)
{
    uart1_begin(9600);
    pinMode(8, INPUT_PULLUP);
    uart1_println("ardupic digital read demo");
}

void loop(void)
{
    uart1_print("pin8=");
    uart1_print_int(digitalRead(8));
    uart1_println("");
    delay(200);
}
