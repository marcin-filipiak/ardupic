/*
 * pic18f2550_bits.h - bit maski rejestrow PIC18F2550 dla core Arduino.
 * Wartosci zgodne z p18f2550.inc (gputils) / datasheet DS39632E.
 */
#ifndef _PIC18F2550_BITS_H
#define _PIC18F2550_BITS_H

/* ---- ADCON0 ---- */
#define ADCON0_ADON  0
#define ADCON0_GO    1
#define ADCON0_CHS0  2
#define ADCON0_CHS1  3
#define ADCON0_CHS2  4
#define ADCON0_CHS3  5

/* ---- ADCON1: VCFG1:VCFG0 (5:4), PCFG3:0 (3:0) ---- */
#define ADCON1_PCFG0 0
#define ADCON1_PCFG1 1
#define ADCON1_PCFG2 2
#define ADCON1_PCFG3 3
#define ADCON1_VCFG0 4
#define ADCON1_VCFG1 5

#define ADCON1_PCFG_ALL_DIG (0x0F)
#define ADCON1_PCFG_ALL_ANA (0x00)

/* ---- ADCON2 ---- */
#define ADCON2_ADCS0  0
#define ADCON2_ADCS1  1
#define ADCON2_ADCS2  2
#define ADCON2_ACQT0  3
#define ADCON2_ACQT1  4
#define ADCON2_ACQT2  5
#define ADCON2_ADFM   7

/* ---- INTCON ---- */
#define INTCON_INT0IF 1
#define INTCON_TMR0IF 2
#define INTCON_INT0IE 4
#define INTCON_TMR0IE 5
#define INTCON_PEIE   6
#define INTCON_GIE    7

/* ---- INTCON2 ---- */
#define INTCON2_INTEDG0 6
#define INTCON2_INTEDG1 5
#define INTCON2_INTEDG2 4
#define INTCON2_NOT_RBPU 7

/* ---- INTCON3 ---- */
#define INTCON3_INT1IF 0
#define INTCON3_INT2IF 1
#define INTCON3_INT1IE 3
#define INTCON3_INT2IE 4

/* ---- T0CON ---- */
#define T0CON_PSA     3
#define T0CON_T0SE    4
#define T0CON_T0CS    5
#define T0CON_T08BIT  6
#define T0CON_TMR0ON  7

/* ---- T2CON ---- */
#define T2CON_T2CKPS0 0
#define T2CON_T2CKPS1 1
#define T2CON_TMR2ON  2

/* ---- CCP1CON ---- */
#define CCP1CON_CCP1M0 0
#define CCP1CON_CCP1M1 1
#define CCP1CON_CCP1M2 2
#define CCP1CON_CCP1M3 3
#define CCP1CON_DC1B0  4
#define CCP1CON_DC1B1  5

/* ---- PIR1 ---- */
#define PIR1_TMR1IF 0
#define PIR1_TXIF   4
#define PIR1_RCIF   5
#define PIR1_ADIF   6

/* ---- PIE1 ---- */
#define PIE1_TMR1IE 0
#define PIE1_TXIE   4
#define PIE1_RCIE   5
#define PIE1_ADIE   6

/* ---- TXSTA ---- */
#define TXSTA_TX9D  0
#define TXSTA_TRMT  1
#define TXSTA_BRGH  2
#define TXSTA_SENDB 3
#define TXSTA_SYNC  4
#define TXSTA_TXEN  5
#define TXSTA_TX9   6
#define TXSTA_CSRC  7

/* ---- RCSTA ---- */
#define RCSTA_RX9D  0
#define RCSTA_OERR  1
#define RCSTA_FERR  2
#define RCSTA_ADDEN 3
#define RCSTA_CREN  4
#define RCSTA_SREN  5
#define RCSTA_RX9   6
#define RCSTA_SPEN  7

/* ---- BAUDCON ---- */
#define BAUDCON_ABDEN 0
#define BAUDCON_WUE   1
#define BAUDCON_BRG16 3
#define BAUDCON_TXCKP 4
#define BAUDCON_RXDTP 5
#define BAUDCON_RCIDL 6
#define BAUDCON_ABDOVF 7

/* ---- OSCCON ---- */
#define OSCCON_IOFS  2
#define OSCCON_IRCF0 4
#define OSCCON_IRCF1 5
#define OSCCON_IRCF2 6
#define OSCCON_IDLEN 7

#endif