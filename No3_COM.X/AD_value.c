/******************************************************************************************
 * AD_value.c
 * ver2.00
 * Tetsuya Kaku
 *=========================================================================================
 * AD変換ソースファイル
 *
 *=========================================================================================
 * ・ver1.00 || 2016/01/14 || Tetsuya Kaku
 *   初版
 * ・ver2.00 || 2016/03/04 || Hirofumi Hamada
 *   bit_shift()のアルゴリズム変更
 *=========================================================================================
 * PIC16F877A
 * MPLAB X IDE(v3.10/Win)
 * XC8 (v1.35/Win)
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/
#include <xc.h>
#include <pic16f877a.h>
#include "pic_clock.h"
#include "AD_value.h"
#include "pic_setting.h"
#include <stdio.h>  //2017/06/06



/* A/Dコン値受信用構造体 */
typedef struct
{
    uint8_t my_adres_h;
    uint8_t my_adres_l;
} adres_t;


/* static関数のプロトタイプ宣言 */
static void bit_shift(uint16_t *p_ad_value, adres_t *p_adres);


/*=====================================================
 * @brief
 *     ADコンバータ設定関数
 * @param
 *     なし
 * @return
 *     なし
 * @note
 *     ADコンバータするピンを要確認
 *===================================================*/
void ad_con_init(void)
{
    
    PORTAbits.RA0 = 0;
    PORTAbits.RA3 = 0;
    TRISAbits.TRISA0 = 1;   //RA0を入力ピンにしてVref＋読み取りピン
    TRISAbits.TRISA3 = 1;   //RA3を入力ピンにしてAD読み取りピン 
    ADCON1 = 0b00000001;
}


/*-----------------------------------------------------
 * @brief
 *     ADRESHとADRESLのデータを結合する
 * @param
 *     p_ad_value:変換値を格納する2Byteの変数へのポインタ
 *     p_adres   :A/D変換のrowデータへのポインタ
 * @return
 *     void
 * @note
 *     AD_value.c内で使用する関数なのでstatic関数
 *---------------------------------------------------*/
static void bit_shift(uint16_t *p_ad_value, adres_t *p_adres)
{
    const uint8_t bit_mask_for_l = 0b00000011;

    *p_ad_value = (uint16_t)(p_adres->my_adres_h);                   // ex. ad_value = 0b0000000011111111
    *p_ad_value <<= 2;                                               // ex. ad_value = 0b0000001111111100
    *p_ad_value |= (bit_mask_for_l & (p_adres->my_adres_l >> 6));    // ex. ad_value = 0b0000001111111111
}


/*=====================================================
 * @brief
 *     AD変換する関数
 * @param
 *     なし
 * @return
 *     AD変換した電圧値(V)
 * @note
 *     AD変換する関数
 *===================================================*/
double get_adcon(void)
{
    /* A/D値用変数 */
    uint16_t ad_value = 0b0000000000;
    double voltage_value = 0.0;
    adres_t my_adres;

    /* A/Dコンを有効化 */
    ADCON0 = 0b10000001;
    ADCON0bits.GO_DONE = 1;

    /* A/Dコン終了まで待つ */
    while(ADCON0bits.GO_DONE){;}

    /* ADRESHとADRESLからデータを取得し結合する */
    my_adres.my_adres_h = ADRESH;
    my_adres.my_adres_l = ADRESL;
    bit_shift(&ad_value, &my_adres);
    
    //printf("ad_value1 = %d\r\n",ad_value);
    voltage_value = (double)(ad_value * 0.00488759); //[V]に変換
    //printf("ad_value2 = %f\r\n",voltage_value);
    
    return voltage_value;
}
