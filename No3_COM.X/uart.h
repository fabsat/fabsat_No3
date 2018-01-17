/******************************************************************************************
 * uart.h
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用UARTヘッダファイル
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/28 || Hirofumi Hamada
 *   初版
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(V3.10/Ubuntu)
 * XC8 (V1.35/Linux)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/

#ifndef _UART_H
#define	_UART_H

#include <xc.h>
#include "pic_setting.h"


/* Setting Baudrate */
#define BAUDRATE    (9600)       // 9.6kbps


/* TXSTA Register Mask */
#define TXSTA_TX9D  (1 << 0)
#define TXSTA_TRMT  (1 << 1)
#define TXSTA_BRGH  (1 << 2)
#define TXSTA_SENDB (1 << 3)
#define TXSTA_SYNC  (1 << 4)
#define TXSTA_TXEN  (1 << 5)
#define TXSTA_TX9   (1 << 6)
#define TXSTA_CSRC  (1 << 7)


/* RCSTA Register Mask */
#define RCSTA_RX9D  (1 << 0)
#define RCSTA_OERR  (1 << 1)
#define RCSTA_FERR  (1 << 2)
#define RCSTA_ADDEN (1 << 3)
#define RCSTA_CREN  (1 << 4)
#define RCSTA_SREN  (1 << 5)
#define RCSTA_RX9   (1 << 6)
#define RCSTA_SPEN  (1 << 7)


/* Baud Rate Control Register Mask */
#define BAUDCTL_ABDEN  (1 << 0)
#define BAUDCTL_WUE    (1 << 1)
#define BAUDCTL_BRG16  (1 << 3)
#define BAUDCTL_SCKP   (1 << 4)
#define BAUDCTL_RCIDL  (1 << 6)
#define BAUDCTL_ABDOVF (1 << 7)


/* Calculate SPBRG */
#define SPBRG_DATA ((unsigned char)((_XTAL_FREQ / BAUDRATE / 16) - 1))    // must set BRGH = 1


/* Prototype of Function */
void uart_init(void);
void put_char(uint8_t byte_data);
void put_string(uint8_t* str);
uint8_t get_char(void);

#endif	/* UART_H */
