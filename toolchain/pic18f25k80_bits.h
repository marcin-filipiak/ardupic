/*
 * pic18f25k80_bits.h - bit masek rejestrów PIC18F25K80 dla core Arduino.
 * Wartości zgodne z p18f25k80.inc (gputils) / datasheet DS39977.
 */
#ifndef _PIC18F25K80_BITS_H
#define _PIC18F25K80_BITS_H

/* ---- ADCON0 ---- */
#define ADCON0_ADON 0
#define ADCON0_GO   1
#define ADCON0_CHS0 2
#define ADCON0_CHS1 3
#define ADCON0_CHS2 4
#define ADCON0_CHS3 5
#define ADCON0_CHS4 6

/* ---- ADCON2 ---- */
#define ADCON2_ADCS0 0
#define ADCON2_ACQT0 3
#define ADCON2_ACQT1 4
#define ADCON2_ADFM  7

/* ---- INTCON ---- */
#define INTCON_RBIF   0
#define INTCON_INT0IF 1
#define INTCON_TMR0IF 2
#define INTCON_RBIE   3
#define INTCON_INT0IE 4
#define INTCON_TMR0IE 5
#define INTCON_PEIE   6
#define INTCON_GIE    7

/* ---- INTCON2 ---- */
#define INTCON2_RBIP    0
#define INTCON2_INT3IP  1
#define INTCON2_TMR0IP  2
#define INTCON2_INTEDG3 3
#define INTCON2_INTEDG2 4
#define INTCON2_INTEDG1 5
#define INTCON2_INTEDG0 6
#define INTCON2_NOT_RBPU 7

/* ---- INTCON3 ---- */
#define INTCON3_INT1IF 0
#define INTCON3_INT2IF 1
#define INTCON3_INT3IF 2
#define INTCON3_INT1IE 3
#define INTCON3_INT2IE 4
#define INTCON3_INT3IE 5
#define INTCON3_INT1IP 6
#define INTCON3_INT2IP 7

/* ---- T0CON ---- */
#define T0CON_T0PS0  0
#define T0CON_T0PS1  1
#define T0CON_T0PS2  2
#define T0CON_PSA    3
#define T0CON_T0SE   4
#define T0CON_T0CS   5
#define T0CON_T08BIT 6
#define T0CON_TMR0ON 7

/* ---- T2CON ---- */
#define T2CON_T2CKPS0 0
#define T2CON_T2CKPS1 1
#define T2CON_TMR2ON  2

/* ---- ECCP1CON ---- */
#define ECCP1CON_CCP1M0 0
#define ECCP1CON_CCP1M1 1
#define ECCP1CON_CCP1M2 2
#define ECCP1CON_CCP1M3 3
#define ECCP1CON_DC1B0  4
#define ECCP1CON_DC1B1  5

/* ---- PIR1 ---- */
#define PIR1_TMR1IF 0
#define PIR1_TXIF   4
#define PIR1_RCIF   5
#define PIR1_ADIF   6

/* ---- PIE1 ---- */
#define PIE1_TXIE 4
#define PIE1_RCIE 5
#define PIE1_ADIE 6

/* ---- TXSTA1 ---- */
#define TXSTA1_TX9D  0
#define TXSTA1_TRMT  1
#define TXSTA1_BRGH  2
#define TXSTA1_SENDB 3
#define TXSTA1_SYNC  4
#define TXSTA1_TXEN  5
#define TXSTA1_TX9   6
#define TXSTA1_CSRC  7

/* ---- RCSTA1 ---- */
#define RCSTA1_RX9D 0
#define RCSTA1_OERR 1
#define RCSTA1_FERR 2
#define RCSTA1_ADDEN 3
#define RCSTA1_CREN 4
#define RCSTA1_SREN 5
#define RCSTA1_RX9  6
#define RCSTA1_SPEN 7

/* ---- BAUDCON1 ---- */
#define BAUDCON1_ABDEN 0
#define BAUDCON1_WUE   1
#define BAUDCON1_BRG16 3

#endif