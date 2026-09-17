#include <Arduino.h>
#include "can.h"

/* ---- masek bitowe (SDCC nie generuje nazw bitowych dla CAN) -------- */
#define CANSTAT_OPMODE_MASK  0xE0
#define CANCON_REQOP_CONFIG  0x80
#define CANCON_REQOP_NORMAL  0x00
#define ECANCON_MDSEL_MASK   0xC0
#define ECANCON_MDSEL_LEGACY 0x00

#define TXBnCON_TXREQ  0x08
#define TXBnCON_TXERR  0x10
#define TXBnCON_TXLARB 0x20
#define TXBnCON_TXABT  0x40
#define TXBnCON_TXBIF  0x80

#define RXBnCON_RXFUL   0x80
#define RXBnCON_RXM_MASK 0x60
#define RXBnCON_RXM_ALL  0x60

#define PIR5_RXB0IF     0x01
#define PIR5_RXB1IF     0x02
#define PIE5_RXB0IE     0x01
#define PIE5_RXB1IE     0x02

#define DLC_MASK        0x0F
#define DLC_RTR         0x40
#define SIDL_EXIDE      0x08

/* ---- tablica predkosci (@ F_CPU = 16 MHz, bit 16 TQ) ---------------- */
typedef struct {
    unsigned long baud;
    unsigned char brg1;
    unsigned char brg2;
    unsigned char brg3;
} can_timing_t;

static const can_timing_t can_timing[] = {
    { CAN_SPEED_125,  0x03, 0x9E, 0x03 },
    { CAN_SPEED_250,  0x01, 0x9E, 0x03 },
    { CAN_SPEED_500,  0x00, 0x9E, 0x03 },
    { CAN_SPEED_1000, 0x00, 0x8B, 0x01 },
};

static volatile can_message_t can_rx[CAN_RX_QUEUE];
static volatile unsigned char can_rx_head;
static volatile unsigned char can_rx_tail;

/* ------------------------------------------------------------------ */

static void can_tx_load(unsigned long id, uint8_t extended,
                        const uint8_t *data, uint8_t len, uint8_t rtr)
{
    TXB2CON = 0;

    if (extended) {
        TXB2SIDH = (unsigned char)(id >> 21);
        TXB2SIDL = (unsigned char)(((unsigned long)(id >> 18) & 0x07UL) << 5)
                 | (unsigned char)((id >> 16) & 0x03UL)
                 | SIDL_EXIDE;
        TXB2EIDH = (unsigned char)((id >> 8) & 0xFFUL);
        TXB2EIDL = (unsigned char)(id & 0xFFUL);
    } else {
        TXB2SIDH = (unsigned char)((id >> 3) & 0xFFUL);
        TXB2SIDL = (unsigned char)((id & 0x07UL) << 5);
        TXB2EIDH = 0;
        TXB2EIDL = 0;
    }

    TXB2DLC = (rtr ? DLC_RTR : 0) | (len & DLC_MASK);

    if (!rtr) {
        switch (len) {
        case 8: TXB2D7 = data[7];
        case 7: TXB2D6 = data[6];
        case 6: TXB2D5 = data[5];
        case 5: TXB2D4 = data[4];
        case 4: TXB2D3 = data[3];
        case 3: TXB2D2 = data[2];
        case 2: TXB2D1 = data[1];
        case 1: TXB2D0 = data[0];
            break;
        default:
            break;
        }
    }
}

/* ---- obsluga przerwania RX (wolana z isr.c) ------------------------ */
void can_irq(void)
{
    unsigned char fl = PIR5 & 0x03;
    volatile can_message_t *m;
    unsigned char next, n, sidh, sidl;

    if (fl == 0)
        return;
    PIR5 = (unsigned char)(PIR5 & 0xFC);   /* skasuj RXB0IF/RXB1IF */

    /* RXB0 (najstarszy, wyzszy priorytet) */
    if (fl & PIR5_RXB0IF) {
        sidh = RXB0SIDH;
        sidl = RXB0SIDL;
        m = &can_rx[can_rx_head];
        m->extended = (uint8_t)((sidl & SIDL_EXIDE) ? 1 : 0);
        if (m->extended) {
            m->id  = ((unsigned long)RXB0EIDH << 8) | RXB0EIDL;
            m->id |= ((unsigned long)sidh << 21);
            m->id |= ((unsigned long)(sidl & 0xE0U) << 13);
            m->id |= ((unsigned long)(sidl & 0x03U) << 16);
        } else {
            m->id = ((unsigned long)sidh << 3) | (unsigned long)(sidl >> 5);
        }
        n = (uint8_t)(RXB0DLC & DLC_MASK);
        if (n > CAN_MAX_DATA)
            n = CAN_MAX_DATA;
        m->length = n;
        m->remote = (uint8_t)((RXB0DLC & DLC_RTR) ? 1 : 0);
        switch (n) {
        case 8: m->data[7] = RXB0D7;
        case 7: m->data[6] = RXB0D6;
        case 6: m->data[5] = RXB0D5;
        case 5: m->data[4] = RXB0D4;
        case 4: m->data[3] = RXB0D3;
        case 3: m->data[2] = RXB0D2;
        case 2: m->data[1] = RXB0D1;
        case 1: m->data[0] = RXB0D0;
            break;
        default:
            break;
        }
        RXB0CON &= (unsigned char)~RXBnCON_RXFUL;
        next = (unsigned char)((can_rx_head + 1U) & (CAN_RX_QUEUE - 1U));
        if (next != can_rx_tail)
            can_rx_head = next;
    }

    /* RXB1 */
    if (fl & PIR5_RXB1IF) {
        sidh = RXB1SIDH;
        sidl = RXB1SIDL;
        m = &can_rx[can_rx_head];
        m->extended = (uint8_t)((sidl & SIDL_EXIDE) ? 1 : 0);
        if (m->extended) {
            m->id  = ((unsigned long)RXB1EIDH << 8) | RXB1EIDL;
            m->id |= ((unsigned long)sidh << 21);
            m->id |= ((unsigned long)(sidl & 0xE0U) << 13);
            m->id |= ((unsigned long)(sidl & 0x03U) << 16);
        } else {
            m->id = ((unsigned long)sidh << 3) | (unsigned long)(sidl >> 5);
        }
        n = (uint8_t)(RXB1DLC & DLC_MASK);
        if (n > CAN_MAX_DATA)
            n = CAN_MAX_DATA;
        m->length = n;
        m->remote = (uint8_t)((RXB1DLC & DLC_RTR) ? 1 : 0);
        switch (n) {
        case 8: m->data[7] = RXB1D7;
        case 7: m->data[6] = RXB1D6;
        case 6: m->data[5] = RXB1D5;
        case 5: m->data[4] = RXB1D4;
        case 4: m->data[3] = RXB1D3;
        case 3: m->data[2] = RXB1D2;
        case 2: m->data[1] = RXB1D1;
        case 1: m->data[0] = RXB1D0;
            break;
        default:
            break;
        }
        RXB1CON &= (unsigned char)~RXBnCON_RXFUL;
        next = (unsigned char)((can_rx_head + 1U) & (CAN_RX_QUEUE - 1U));
        if (next != can_rx_tail)
            can_rx_head = next;
    }
}

uint8_t can_begin(unsigned long baud)
{
    uint8_t i;

    if (baud == 0UL)
        baud = CAN_SPEED_250;

    for (i = 0; i < (uint8_t)(sizeof(can_timing) / sizeof(can_timing[0])); i++) {
        if (can_timing[i].baud == baud)
            break;
    }
    if (i == (uint8_t)(sizeof(can_timing) / sizeof(can_timing[0])))
        return 0;                       /* nieznana predkosc */

    /* wejdz w tryb konfiguracji */
    CANCON = CANCON_REQOP_CONFIG;
    while ((CANSTAT & CANSTAT_OPMODE_MASK) != CANCON_REQOP_CONFIG)
        ;

    ECANCON &= (unsigned char)~ECANCON_MDSEL_MASK;  /* tryb Legacy */

    BRGCON1 = can_timing[i].brg1;
    BRGCON2 = can_timing[i].brg2;
    BRGCON3 = can_timing[i].brg3;

    /* odbieraj wszystko (filtry i maski pomijane) */
    RXB0CON = RXBnCON_RXM_ALL;
    RXB1CON = RXBnCON_RXM_ALL;

    PIR5 = 0;                           /* wyczysc flagi CAN */
    PIE5 = (unsigned char)(PIE5_RXB0IE | PIE5_RXB1IE);

    /* wroc do trybu normalnego */
    CANCON = CANCON_REQOP_NORMAL;
    while ((CANSTAT & CANSTAT_OPMODE_MASK) != CANCON_REQOP_NORMAL)
        ;

    return 1;
}

static uint8_t can_tx(unsigned long id, uint8_t extended,
                      const uint8_t *data, uint8_t len, uint8_t rtr)
{
    unsigned int guard;

    if (len > CAN_MAX_DATA)
        len = CAN_MAX_DATA;

    /* poczekaj az poprzednia ramka z TXB2 opusci bufor */
    guard = 0;
    while (TXB2CON & TXBnCON_TXREQ) {
        if (++guard == 0U)
            return 0;
    }

    can_tx_load(id, extended, data, len, rtr);

    /* zadaj transmisje i czekaj na zakonczenie */
    TXB2CON = TXBnCON_TXREQ;
    guard = 0;
    while (TXB2CON & TXBnCON_TXREQ) {
        if (++guard == 0U)
            return 0;
    }

    return (TXB2CON & (TXBnCON_TXABT | TXBnCON_TXERR)) ? 0 : 1;
}

uint8_t can_send_std(unsigned int id, const uint8_t *data, uint8_t len)
{
    return can_tx((unsigned long)id, 0, data, len, 0);
}

uint8_t can_send_ext(unsigned long id, const uint8_t *data, uint8_t len)
{
    return can_tx(id, 1, data, len, 0);
}

uint8_t can_send(const can_message_t *msg)
{
    return can_tx(msg->id, msg->extended, msg->data, msg->length, msg->remote);
}

uint8_t can_available(void)
{
    return (uint8_t)((can_rx_head - can_rx_tail) & (CAN_RX_QUEUE - 1U));
}

uint8_t can_read(can_message_t *msg)
{
    uint8_t ok = 0;
    const volatile can_message_t *m;
    uint8_t i;

    INTCON &= (unsigned char)~(1U << INTCON_GIE);
    if (can_rx_head != can_rx_tail) {
        m = &can_rx[can_rx_tail];
        msg->id = m->id;
        msg->length = m->length;
        msg->extended = m->extended;
        msg->remote = m->remote;
        for (i = 0; i < m->length; i++)
            msg->data[i] = m->data[i];
        can_rx_tail = (unsigned char)((can_rx_tail + 1U) & (CAN_RX_QUEUE - 1U));
        ok = 1;
    }
    INTCON |= (1U << INTCON_GIE);
    return ok;
}