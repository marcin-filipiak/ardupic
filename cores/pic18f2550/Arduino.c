#include <Arduino.h>

/* PIC18F2550 configuration:
   HS-PLL with a 20 MHz crystal -> PLL x24 = 96 MHz, CPU = 96/2 = 48 MHz,
   USB = 96/2 = 48 MHz (full speed, 20 MHz crystal required),
   WDT off, XINST off, PORTB analog disabled by default,
   internal USB regulator (VUSB) enabled */
#pragma config PLLDIV=5, CPUDIV=OSC1_PLL2, USBDIV=2
#pragma config FOSC=HSPLL_HS, FCMEN=OFF, IESO=OFF
#pragma config PWRT=ON, BOR=OFF, BORV=0, VREGEN=ON
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