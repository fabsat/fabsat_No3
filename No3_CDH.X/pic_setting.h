/******************************************************************************************
 * pic_setting.h
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * PIC16F877A用クロック周波数定義ヘッダファイル
 *
 *=========================================================================================
 * ・ver1.00 || 初版 || 2015/12/28 || Hirofumi Hamada
 *   初版動作
 * ・ver2.00 || unsigned追加 || 2016/02/09 || Hirofumi Hamada
 *   unsinged型の追加
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(V3.10/Ubuntu)
 * XC8 (V1.35/Linux)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/

#ifndef _PIC_SETTING_H
#define _PIC_SETTING_H


/* クロックの周波数定義 */
#define _XTAL_FREQ 20000000


/* unsigned型の型名変換 */
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;


/* bool型の定義 (FALSE = 0, TRUE = 1) */
typedef enum
{
	FALSE,
	TRUE,
} bool_t;

#endif  /* _PIC_SETTING_H */
