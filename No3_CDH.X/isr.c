
/******************************************************************************************
 * isr.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用割り込み関数ファイル(COM)
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/31 || Hirofumi Hamada
 *   初版作成
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(V3.10/Ubuntu)
 * XC8 (V1.35/Linux)
 * 
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/
#include <xc.h>
#include "pic_setting.h"
#include "spi_slave.h"
#include "protocol.h"
#include "uart.h"
#include "isr.h"


/*=====================================================
 * ISR Function
 *===================================================*/
void interrupt isr(void)
{    
    uint8_t isr_spi_buf;

    /* SPI割込み */
    if (PIR1bits.SSPIF == 1)
    {
        isr_spi_buf = SSPBUF; // プリアンブルデータを取り除く
        SSPBUF      = 0xff;   // SSPBUFに受信用のダミーデータを格納

        /* Slave命令コードを受け取る */
        spi_slave_receive(&isr_spi_buf);

        /* Slave命令コードに従ってサブルーチンへ分岐 */
        switch(isr_spi_buf)
        {
            case RX_MODE:
                receive_packet();
                break;
            case TX_MODE:
                send_packet();
                break;
            case FLAG_CHECK_MODE:
                check_flag();
                break;
            default:
                break;
        }

        /* 割込みフラグをクリア */
        PIR1bits.SSPIF = 0;
    }
}


