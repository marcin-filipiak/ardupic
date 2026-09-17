#ifndef Can_h
#define Can_h

#include <Arduino.h>

/* ---- CAN for the PIC18F25K80 (ECAN, Legacy mode) --------------------
 * Pins: CANTX = RC6 (Arduino pin 22), CANRX = RC7 (Arduino pin 23).
 * NOTE: same pins as EUSART1 (uart1_*) - CAN and UART1 are mutually
 * exclusive. Do not use both at the same time.
 *
 * Reception is handled in the interrupt (RXB0/RXB1 -> queue);
 * transmission is blocking (uses TXB2).
 * The bit timing assumes F_CPU = 16 MHz (matching the core configuration).
 */

#define CAN_SPEED_125  125000UL
#define CAN_SPEED_250  250000UL
#define CAN_SPEED_500  500000UL
#define CAN_SPEED_1000 1000000UL

#define CAN_MAX_DATA        8
#define CAN_RX_QUEUE        8   /* length of the received-frame queue */

typedef struct {
    unsigned long id;   /* 11-bit (standard) or 29-bit (extended) */
    uint8_t length;     /* 0..8 */
    uint8_t data[8];
    uint8_t extended;   /* 1 = 29-bit frame */
    uint8_t remote;     /* 1 = remote frame (RTR) */
} can_message_t;

/* enables CAN at the given baud rate; returns 1 on success, 0 on unknown baud */
uint8_t can_begin(unsigned long baud);

/* blocking send of a standard / extended frame; 1 = ok, 0 = error */
uint8_t can_send_std(unsigned int id, const uint8_t *data, uint8_t len);
uint8_t can_send_ext(unsigned long id, const uint8_t *data, uint8_t len);
uint8_t can_send(const can_message_t *msg);

/* number of frames waiting in the receive queue */
uint8_t can_available(void);

/* pops the oldest frame; returns 1 if one was read, 0 if the queue is empty */
uint8_t can_read(can_message_t *msg);

/* CAN interrupt handler (called from isr.c) */
void can_irq(void);

#endif