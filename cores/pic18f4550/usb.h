/*
 * usb.h - USB CDC (virtual serial) for PIC18F2550/4550 (SDCC).
 *
 * Polled-only USB engine (non ping-pong, "Mode 0"), BDT at 0x400:
 *   bdt[EP*2 + DIR]   (DIR: 0=OUT,1=IN)
 *
 * API (call usbcdc_poll() from loop() or rely on the blocking I/O
 * functions which service the USB engine themselves):
 *   usbcdc_begin()              - attach module, wait for enumeration
 *   usbcdc_poll()               - service USB events
 *   usbcdc_available()/read()   - host -> device FIFO
 *   usbcdc_write()/print*()     - device -> host
 */
#ifndef _USB_H
#define _USB_H

#include <stdint.h>

void usbcdc_begin(void);
void usbcdc_poll(void);

void usbcdc_write(unsigned char c);
void usbcdc_print(const char *s);
void usbcdc_println(const char *s);
void usbcdc_print_ulong(unsigned long v);
void usbcdc_print_int(long v);

int usbcdc_read(void);
int usbcdc_available(void);

#endif /* _USB_H */