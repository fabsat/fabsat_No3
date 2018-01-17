/******************************************************************************************
 * spi_master.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用SPI通信(Master)ソースファイル
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/27 || Hirofumi Hamada
 *   初版作成
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(V3.10/Ubuntu)
 * XC8 (V1.35/Linux)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/

#include <xc.h>
#include "spi_master.h"
#include "pic_setting.h"
#include "protocol.h"


/*=====================================================
 * @breif
 *     SPI Master初期化関数
 * @param
 *     なし
 * @return
 *     なし
 * @note
 *     Pin43:SDO(RC5)
 *     Pin42:SDI(RC4/SDA)
 *     Pin37:SCK(RC3/SCL)
 *===================================================*/
void spi_master_start(void)
{
    /* SPI pin I/O Configuration */
    TRISCbits.TRISC3 = 0;              // RC3 is SCK -> OUTPUT
    TRISCbits.TRISC4 = 1;              // RC4 is SDI -> INPUT
    TRISCbits.TRISC5 = 0;              // RC5 is SDO -> OUTPUT

    /* SS pin configure OUTPUT */
    SS_OBC2_TRIS = 0;      // RC1 is SS  -> OUTPUT
    SS_COM_TRIS  = 0;
    SS_POW_TRIS  = 0;

    /* Allow Programming of SPI configuring */
    SSPCONbits.SSPEN = 0;

    /*  SPI Mode Setup */
    SSPSTATbits.SMP = 0;    // Input data sampled at middle of data output time
    SSPCONbits.CKP  = 0;    // Idle state for clock is a low level
    SSPSTATbits.CKE = 0;    // Transmit occurs on transition from idle to active clock state
    
    /* SPI Master mode, clock = FOSC/4(Maximum speed) */
    SSPCONbits.SSPM3 = 0;          
    SSPCONbits.SSPM2 = 0;          
    SSPCONbits.SSPM1 = 0;          
    SSPCONbits.SSPM0 = 0;

    /* SS_PIN set HIGH */
    SS_OBC2 = 1;
    SS_COM  = 1;
    SS_POW  = 1;
   
    /* End SPI programming and Start serial port */
    SSPCONbits.SSPEN = 1;
}



/*=====================================================
 * @brief
 *     SPI Masterデータ送信関数(1Byte)
 * @param
 *     destination:通信の相手先を選択
 *     data       :送信データ
 * @return
 *     void:
 * @note
 *     none
 *===================================================*/
void spi_master_send(destination_t destination, uint8_t data)
{
    uint8_t buf;
 
     /* Read data to dummy */
    buf = SSPBUF;

    /* Slave SelectピンをLowにする */    
    switch(destination)
    {
        case OBC2:
            SS_OBC2 = 0;
            break;
        case COM:
            SS_COM  = 0;
            break;
        case POW:
            SS_POW  = 0;
            break;
        default:
            break;
    }
    
    /* Set dummy data to SSPBUF ,SPI START */
    SSPBUF = data;

    /* Wait for receive finish */
    while(SSPSTATbits.BF == 0)
    {
        ;
    }

    /* Slave Select -> High */
    switch(destination)
    {
        case OBC2:
            SS_OBC2 = 1;
            break;
        case COM:
            SS_COM  = 1; 
            break;
        case POW:
            SS_POW  = 1;
            break;
        default:
            break;
    }
    
    /* Read data to dummy */
    buf = SSPBUF;
}


/*=====================================================
 * @brief
 *     SPI Masterデータ受信関数(1Byte)
 * @param
 *     destination:通信の相手先を選択
 *     p_get      :受信データを格納するポインタ
 * @return
 *     void:
 * @note
 *     none
 *===================================================*/
void spi_master_receive(destination_t destination, uint8_t *p_get)
{
    uint8_t buf;

    /* SSPBUFからデータを取り除く */
    buf = SSPBUF;

    /* Slave SelectピンをLowにする */    
    switch(destination)
    {
        case OBC2:
            SS_OBC2 = 0;
            break;
        case COM:
            SS_COM  = 0;
            break;
        case POW:
            SS_POW  = 0;
            break;
        default:
            break;
    }
    
    /* ダミーデータをSSPBUFに格納して通信開始 */
    SSPBUF = 0xff;

    /* SPI通信完了待ち */
    while(SSPSTATbits.BF == 0)
    {
        ;
    }

    /* Slave Select -> High */
    switch(destination)
    {
        case OBC2:
            SS_OBC2 = 1;
            break;
        case COM:
            SS_COM  = 1; 
            break;
        case POW:
            SS_POW  = 1;
            break;
        default:
            break;
    }
    
    /* 引数で受け取ったアドレスへ受信データを格納 */
    *p_get = SSPBUF;
}

