#ifndef Can_h
#define Can_h

#include <Arduino.h>

/* ---- CAN dla PIC18F25K80 (ECAN, tryb Legacy) --------------------------
 * Piny: CANTX = RC6 (Arduino pin 22), CANRX = RC7 (Arduino pin 23).
 * UWAGA: te same piny co EUSART1 (uart1_*) - CAN i UART1 sa wzajemnie
 * wylaczne. Nie wolno uzywac obu naraz.
 *
 * Przetwarzanie odbioru odbywa sie w przerwaniu (RXB0/RXB1 -> kolejka);
 * transmisja jest blokujaca (maks. TXB2).
 * Czas bazowy zaklada F_CPU = 16 MHz (zgodnie z konfiguracja core'a).
 */

#define CAN_SPEED_125  125000UL
#define CAN_SPEED_250  250000UL
#define CAN_SPEED_500  500000UL
#define CAN_SPEED_1000 1000000UL

#define CAN_MAX_DATA        8
#define CAN_RX_QUEUE        8   /* dlugosc kolejki odebranych ramek */

typedef struct {
    unsigned long id;   /* 11-bit (standard) lub 29-bit (rozszerzony) */
    uint8_t length;     /* 0..8 */
    uint8_t data[8];
    uint8_t extended;   /* 1 = ramka 29-bit */
    uint8_t remote;     /* 1 = ramka zdalna (RTR) */
} can_message_t;

/* wlacza CAN na wybranej predkosci; zwraca 1 gdy ok, 0 gdy nieznane baud */
uint8_t can_begin(unsigned long baud);

/* blokujace wyslanie ramki standardowej / rozszerzonej; 1 = ok, 0 = blad */
uint8_t can_send_std(unsigned int id, const uint8_t *data, uint8_t len);
uint8_t can_send_ext(unsigned long id, const uint8_t *data, uint8_t len);
uint8_t can_send(const can_message_t *msg);

/* liczba ramek czekajacych w kolejce odbiorczej */
uint8_t can_available(void);

/* pobiera najstarsza ramke; zwraca 1 gdy pobrano, 0 gdy kolejka pusta */
uint8_t can_read(can_message_t *msg);

/* obsluga przerwania CAN (wolana z isr.c) */
void can_irq(void);

#endif