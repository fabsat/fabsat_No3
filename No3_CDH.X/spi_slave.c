
/******************************************************************************************
 * spi_slave.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用SPI通信(Slave)ライブラリソースファイル
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/28 || Hirofumi Hamada
 *   初版作成
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(V3.10/Ubuntu)
 * XC8 (V1.35/Linux)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/

#include <xc.h>
#include "spi_slave.h"
#include "pic_setting.h"
#include "uart.h"


/*=====================================================
 * @brief
 *     SPI Slave初期化関数
 * @param
 *     spi_isr:spi割り込みの有効無効を設定
 * @return
 *     なし
 * @note
 *     Pin43:SDO(RC5)
 *     Pin42:SDI(RC4/SDA)
 *     Pin37:SCK(RC3/SCL)
 *     Pin24:SS (RA5/AN4/C2OUT)
 *===================================================*/
void spi_slave_start(spi_isr_set_t spi_isr)
{
    /* SPI pin I/O Configuration */
    TRISC3 = 1;               // RC3 = SCK -> INPUT
    TRISC4 = 1;               // RC4 = SDI -> INPUT
    TRISC5 = 0;               // RC5 = SDO -> OUTPUT
    TRISA5 = 1;               // RA5 = SS  -> INPUT
    ADCON0bits.ADON  = 0;     // RA5 digital Setting
    ADCON1bits.PCFG3 = 0;     // Have to configure to use SS control enable
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG1 = 1;

    /* Allow Programming of SPI configuration */
    SSPCONbits.SSPEN = 0;

    /*  SPI Mode Setup */
    SSPSTATbits.SMP = 0;    // SMP must be cleared when SPI is used in mode
    SSPCONbits.CKP  = 0;    // Idle state for clock is a low level
    SSPSTATbits.CKE = 0;    // Transmit occurs on transition from idle to active clock state
    
    /* SPI Slave mode, clock = SCK pin. SS pin control enabled. */
    SSPCONbits.SSPM3 = 0;
    SSPCONbits.SSPM2 = 1;
    SSPCONbits.SSPM1 = 0;
    SSPCONbits.SSPM0 = 0;

    /* Enable SPI Interrupt */
    if(spi_isr == SPI_ISR_ENABLE)
    {
        PIR1bits.SSPIF  = 0;
        PIE1bits.SSPIE  = 1;     // SPI Interrupt enable
        INTCONbits.PEIE = 1;
        INTCONbits.GIE  = 1;        
    }

    /* End SPI programming and Start serial port */
    SSPCONbits.SSPEN = 1;    // Enables serial port and configures SCK, SDO, SDI, and SS as serial port pins
}


/*=====================================================
 * @brief
 *     SPIデータ送信関数(スレーブ用)
 * @param
 *     data:送信データ(1Byte)
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void spi_slave_send(uint8_t data)
{
    uint8_t buf;

    /* 引数で受け取ったデータを送信用バッファに格納する */
    SSPBUF = data;

    /* 送信完了待ち */
    while(SSPSTATbits.BF == 0)
    {
        ;
    }

    /* 受信したデータを取り除く */
    buf = SSPBUF;

    /* 次の通信に備えダミーデータをセット */
    SSPBUF = 0xff;
}


/*=====================================================
 * @brief
 *     SPIデータ受信関数(1Byte)
 * @param
 *     p_get:受信データを格納する先のポインタ
 * @return
 *     void:
 * @note
 *     none
 *===================================================*/
void spi_slave_receive(uint8_t *p_get)
{
    /* 受信完了待ち */
    while(SSPSTATbits.BF == 0)
    {
        ;
    }
    
    /* 引数で受け取ったアドレスに受信データを格納 */
    *p_get = SSPBUF;

    /* 次の受信に備えダミーデータを格納する */
    SSPBUF = 0xff;
}
