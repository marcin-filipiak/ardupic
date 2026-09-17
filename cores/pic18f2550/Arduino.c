#include <Arduino.h>

/* Konfiguracja PIC18F2550:
   wewn. oscylator 8 MHz (INTOSCIO_EC -> RA6/RA7 jako I/O),
   WDT off, XINST off, PORTB analog domyslnie wylaczony */
#pragma config PLLDIV=5, CPUDIV=OSC1_PLL2, USBDIV=2
#pragma config FOSC=INTOSCIO_EC, FCMEN=OFF, IESO=OFF
#pragma config PWRT=ON, BOR=OFF, BORV=0, VREGEN=OFF
#pragma config WDT=OFF, WDTPS=1
#pragma config CCP2MX=ON, PBADEN=OFF, LPT1OSC=OFF, MCLRE=ON
#pragma config STVREN=ON, LVP=OFF, XINST=OFF, DEBUG=OFF
#pragma config CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF
#pragma config CPB=OFF, CPD=OFF
#pragma config WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF
#pragma config WRTC=OFF, WRTB=OFF, WRTD=OFF
#pragma config EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF
#pragma config EBTRB=OFF

extern void setup(void);
extern void loop(void);

void main(void)
{
    init();
    setup();
    for (;;)
        loop();
}