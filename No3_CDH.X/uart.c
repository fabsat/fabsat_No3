

/******************************************************************************************
 * uart.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用UARTソースファイル
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

#include <xc.h>
#include "uart.h"
#include "pic_setting.h"


/*=====================================================
 * @brief
 *     Initialize uart
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     Pin01:RX(RC7/DT)
 *     Pin44:TX(RC6/CK)
 *     BRGH set -> High Speed Mode
 *==================================================*/
void uart_init(void)
{
    /* Initialize RX, TX pin by TRISC */
    TRISC6 = 0;    // RC6 = TX -> OUTPUT
    TRISC7 = 1;    // RC7 = RX -> INPUT
 
    /* Initialize EUSART */
    SPBRG = SPBRG_DATA;
    TXSTA = (TXSTA_TXEN | TXSTA_BRGH);
    RCSTA = (RCSTA_SPEN | RCSTA_CREN);
}


/*=====================================================
 * @breif
 *     Transmit 1 Byte data
 * @param
 *     byte_data:1byte data to transmit
 * @return
 *     void:
 * @note
 *     none
 *===================================================*/
void put_char(uint8_t byte_data)
{   
    /* Wait until TXREG is empty */
    while(TXIF == 0)
    {
        ;        
    }
    
    /* Write transmitted data */
    TXREG = byte_data;
}


/*=====================================================
 * Transmit String
 *===================================================*/
void put_string(uint8_t *str)
{
    while(*str != '\0')
    {
        put_char(*str);
        str++;
    }
    
    put_char('\0');
}


/*=====================================================
 * Receive 1 Byte
 *===================================================*/
uint8_t get_char(void)
{
    while(RCIF == 0)
    {
        ;        
    }
 
    return RCREG;
}




