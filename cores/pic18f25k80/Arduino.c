#include <Arduino.h>

/* Domyślna konfiguracja PIC18F25K80:
   wewn. oscylator INTIO2 (16 MHz), WDT off, CAN-AFD off, XINST off */
#pragma config FOSC=INTIO2, INTOSCSEL=LOW, SOSCSEL=DIG, RETEN=ON
#pragma config WDTEN=OFF
#pragma config BORPWR=HIGH, BBSIZ=BB1K, MSSPMSK=MSK7, CANMX=PORTC
#pragma config XINST=OFF

extern void setup(void);
extern void loop(void);

void main(void)
{
    init();
    setup();
    for (;;)
        loop();
}