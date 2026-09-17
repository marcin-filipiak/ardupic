#include <Arduino.h>

/* EUSART: TX = RC6 (Arduino pin 22), RX = RC7 (Arduino pin 23).
 * BRGH=0, BRG16=1 -> SPBRG = Fosc/(16*baud) - 1.
 * Supported reliably @ 8 MHz: 4800, 9600, 19200, 38400, 57600.
 */

void uart1_begin(unsigned long baud)
{
    unsigned long div;
    unsigned int spbrg;

    if (baud == 0UL)
        baud = 9600UL;

    div = F_CPU / (16UL * baud);
    if (div == 0UL)
        div = 1UL;
    if (div > 4096UL)
        div = 4096UL;
    spbrg = (unsigned int)(div - 1UL);

    TRISC |= (1U << 7);        /* RC7 = RX input */

    SPBRGH = (unsigned char)(spbrg >> 8);
    SPBRG  = (unsigned char)(spbrg & 0xFF);

    BAUDCON = (1U << BAUDCON_BRG16);  /* 16-bit baud rate generator */
    TXSTA = (1U << TXSTA_TXEN);       /* async 8-bit, TX enabled */
    RCSTA = (1U << RCSTA_SPEN) | (1U << RCSTA_CREN);
}

/* transmit one byte */
void uart1_write(unsigned char c)
{
    while (!(TXSTA & (1U << TXSTA_TRMT)))
        ;
    TXREG = c;
}

/* transmit a NUL-terminated string */
void uart1_print(const char *s)
{
    while (*s)
        uart1_write((unsigned char)*s++);
}

void uart1_println(const char *s)
{
    uart1_print(s);
    uart1_write('\r');
    uart1_write('\n');
}

static void uart1_put_udec(unsigned long v)
{
    char buf[12];
    uint8_t i = 0;
    do {
        buf[i++] = (char)('0' + (v % 10UL));
        v /= 10UL;
    } while (v != 0UL);
    while (i)
        uart1_write((unsigned char)buf[--i]);
}

void uart1_print_ulong(unsigned long v)
{
    uart1_put_udec(v);
}

void uart1_print_int(long v)
{
    if (v < 0L) {
        uart1_write('-');
        uart1_put_udec((unsigned long)(-v));
    } else {
        uart1_put_udec((unsigned long)v);
    }
}

/* receive one byte (blocking) */
int uart1_read(void)
{
    unsigned char c;
    while (uart1_available() == 0)
        ;
    c = RCREG;
    if (RCSTA & (1U << RCSTA_OERR)) {      /* overrun -> clear */
        RCSTA &= ~(1U << RCSTA_CREN);
        RCSTA |= (1U << RCSTA_CREN);
        return -1;
    }
    return (int)c;
}

unsigned char uart1_available(void)
{
    return (PIR1 & (1U << PIR1_RCIF)) ? 1 : 0;
}