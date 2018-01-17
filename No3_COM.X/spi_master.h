/******************************************************************************************
 * spi_master.h
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * SPI通信(マスター側)ヘッダファイル
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/27 || Hirofumi Hamada
 *   初版作成
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(v3.10/Win)
 * XC8 (v1.35/Win)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/
#ifndef _SPI_MASTER_H
#define	_SPI_MASTER_H

#include <xc.h>
#include "pic_setting.h"
#include "protocol.h"


/* Slave Select Pin */
#define SS_OBC2        PORTCbits.RC2
#define SS_COM         PORTCbits.RC0
#define SS_POW         PORTCbits.RC1


/* SS pinのTRISレジスタ */
#define SS_OBC2_TRIS   TRISCbits.TRISC2
#define SS_COM_TRIS    TRISCbits.TRISC0
#define SS_POW_TRIS    TRISCbits.TRISC1



/* Prototype of Function */
/*=====================================================
 * @breif
 *     SPI Masterスタート関数
 * @param
 *     なし
 * @return
 *     なし
 * @note
 *     Pin43:SDO(RC5)
 *     Pin42:SDI(RC4/SDA)
 *     Pin37:SCK(RC3/SCL)
 *===================================================*/
void spi_master_start(void);


/*=====================================================
 * @brief
 *     SPI Masterデータ送信関数(1Byte)
 * @param
 *     destination:通信の相手先を選択
 *     data     :送信データ
 * @return
 *     void:
 * @note
 *     1[ms]で送信完了しなければTIMEOUTとなる
 *===================================================*/
void spi_master_send(destination_t destination, uint8_t data);


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
void spi_master_receive(destination_t destination, uint8_t *p_get);





#endif	/* _SPI_MASTER_H */

