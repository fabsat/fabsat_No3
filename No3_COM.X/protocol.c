/******************************************************************************************
 * protocol.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * 通信プロトコルについてのライブラリ　ソースファル(OBC1)
 *
 *=========================================================================================
 * ・ver1.00 || 2016/08/19 || Hirofumi Hamada
 *   初版作成
 *=========================================================================================
 * PIC16F877A
 * 
 *=========================================================================================
 * Created by fabsat Project(Tokai university Satellite Project[TSP])
 *****************************************************************************************/
#include <xc.h>
#include "pic_setting.h"
#include "protocol.h"
#include "spi_master.h"
#include "uart.h"
#include "uint8_to_string.h"




/******************************************************************************************
 *                                パケット関連の各種定義                                    *
 *****************************************************************************************/
/* Payloadの最大サイズ */
#define MAX_PAYLOAD_SIZE  30
#define MAX_PAYLOAD_INDEX (MAX_PAYLOAD_SIZE - 1) 


/* パケットフォーマット構造体 */
typedef struct
{
    uint8_t data_type;                 // Payloadデータのタイプを示す
    uint8_t payload_len;               // Payloadのデータ長を示す
    uint8_t payload[MAX_PAYLOAD_SIZE]; // Payload本体
    uint8_t data_end_command;          // データ終了コマンド
} packet_format_t;


/******************************************************************************************
 *                                     グローバル変数                                      *
 *****************************************************************************************/
/* このグローバル変数のパケットに各種データを格納していく */
static packet_format_t tx_packet;  // TX:データが送信されたら初期化される 
static packet_format_t rx_packet;  // RX:データがextractされたら初期化される


/* rx_packetの状態を表す変数(TRUE:パケットFULL FALSE:パケットEmpty ) */
bool_t rx_packet_status = FALSE;


/******************************************************************************************
 *                                static関数のプロトタイプ宣言                              *
 *****************************************************************************************/
static void flag_check(destination_t destination);
static void uint8_data_set(uint8_t *p_data, uint8_t data_len);
static void uint16_data_set(uint16_t *p_data, uint8_t data_len);
static void double_data_set(double *p_data, uint8_t data_len);
static void uint16_to_byte_array(uint16_t *p_data, uint8_t *p_data_array);
static void double_to_byte_array(double *p_data, uint8_t *p_data_array);




/******************************************************************************************
 *                                     ライブラリ関数                                      *
 *****************************************************************************************/

/*=====================================================
 * @brief
 *     Payloadに送信データを格納していく
 * @param
 *     p_data      :uint8_t, uint16_t, doubleの
 *                  配列へのポインタ
 *
 *     data_len    :p_dataは何データあるか
 *                  ただし以下のデータ数制限がある
 *                  uint8_tは15データ
 *                  uint16_tは10データ
 *                  doubleは6データ
 *
 *     byte_of_type:格納したいデータ型のバイト数
 *                  uint8_tなら1
 *                  uintt16_tなら2
 *                  doubleなら4
 * @return
 *     Flee space of payload:ペイロードの残り空き容量
 *     0xff:渡されたデータ全ては格納できない
 *          またはbyte_of_dataが適切な値でない
 * @note
 *     最大ペイロードサイズまで格納できる
 *===================================================*/
uint8_t sent_data_set(void *p_data, uint8_t data_len, uint8_t byte_of_type)
{
    /* 渡されたデータを格納する空き容量があるか確認 なければ0xffを返す */
    if((MAX_PAYLOAD_SIZE - tx_packet.payload_len) - (data_len * (byte_of_type + 1)) < 0)
    {
        return 0xff;
    }
        
    /* 各型のデータ格納へ分岐 */
    switch(byte_of_type)
    {
        case 1:
            uint8_data_set((uint8_t *)p_data, data_len);
            break;
        case 2:
            uint16_data_set((uint16_t *)p_data, data_len);
            break;
        case 4:
            double_data_set((double *)p_data, data_len);
            break;
        default:
            return 0xff;
    }
        
    /* 残りのPayload空き容量(Byte)を返す */
    return (uint8_t)(MAX_PAYLOAD_SIZE - tx_packet.payload_len);
}


/*=====================================================
 * @brief
 *     指定したサブシステムにデータを送信する(Master用)
 * @param
 *     destination     :送信の相手先
 *     data_type       :payloadに格納したデータのタイプ
 *     data_end_command:データはまだ継続するかどうか
 * @return
 *     void:
 * @note
 *     この関数実行後にsetしたデータ内容は初期化される
 *===================================================*/
void send_data_master(destination_t destination, data_type_t data_type, data_end_command_t data_end_command)
{
    uint8_t i;
    
    /* データタイプと終了コマンドをパケットに格納 */
    tx_packet.data_type        = (uint8_t)data_type;
    tx_packet.data_end_command = (uint8_t)data_end_command;
    
    /* プリアンブルを送信 */
    spi_master_send(destination, 0b10101010);
    __delay_ms(10);

    /* Slave命令コマンドを送信 */
    spi_master_send(destination, RX_MODE);
    __delay_ms(10);

    /* データタイプを送信 */
    spi_master_send(destination, tx_packet.data_type);
    __delay_ms(10);

    /* ペイロード長を送信 */
    spi_master_send(destination, tx_packet.payload_len);
    __delay_ms(10);

    /* ペイロードを送信 */
    for(i = 0; i < tx_packet.payload_len; i++)
    {
        spi_master_send(destination, tx_packet.payload[i]);
        __delay_ms(10);
    }

    /* 終了コマンドを送信 */
    spi_master_send(destination, tx_packet.data_end_command);
    __delay_ms(10);
    
    /* tx_packetを初期化 */
    tx_packet_clear();
}


/*=====================================================
 * @brief
 *     指定したサブシステムからデータを受信する(Master用)
 * @param
 *     destination:受信の相手先
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void receive_data_master(destination_t destination)
{
    uint8_t i;

    /* Slave側の送信準備完了を待つ */
    flag_check(POW);
    
    /* プリアンブルを送信 */
    spi_master_send(destination, 0b10101010);
    __delay_ms(10);

    /* Slave命令コードを送信 */
    spi_master_send(destination, TX_MODE);
    __delay_ms(10);

    /* データタイプを受信 */
    spi_master_receive(destination, &rx_packet.data_type);
    __delay_ms(10);

    /* ペイロード長を受信 */
    spi_master_receive(destination, &rx_packet.payload_len);
    __delay_ms(10);

    /* ペイロードを受信 */
    for(i = 0; i < rx_packet.payload_len; i++)
    {
        spi_master_receive(destination, &rx_packet.payload[i]);
        __delay_ms(10);
    }

    /* 終了コマンドを受け取る */
    spi_master_receive(destination, &rx_packet.data_end_command);
    __delay_ms(10);

    /* rx_packet_statusをTRUEへ */
    rx_packet_status = TRUE;
}


/*=====================================================
 * @brief
 *     受信パケットからデータを取り出す
 * @param
 *     p_data_type_buf:データタイプを取り出す用のポインタ
 *     p_data_buf     :ペイロードを取り出す用のポインタ
 *     P_data_len_buf :ペイロード長を取り出す用のポインタ
 * @return
 *     void:
 * @note
 *     実行後rx_packetは初期化される
 *     実行後rx_packet_statusはFALSEとなる
 *===================================================*/
void extract_receive_packet(uint8_t *p_data_type_buf, uint8_t *p_data_buf, uint8_t *p_data_len_buf)
{
    uint8_t i;
    
    /* データタイプをコピー */
    *p_data_type_buf = rx_packet.data_type;

    /* ペイロードをコピー */
    for(i = 0; i < rx_packet.payload_len; i++)
    {
        *p_data_buf++ = rx_packet.payload[i];
    }

    /* ペイロード長をコピー */
    *p_data_len_buf = rx_packet.payload_len;

    /* 受信用パケットを初期化する */
    rx_packet_clear();

    /* rx_packet_statusをFALSへ */
    rx_packet_status = FALSE;
}


/*=====================================================
 * @brief
 *     送信用パケットを初期化する
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void tx_packet_clear(void)
{
    uint8_t i;

    /* データタイプをクリア */
    tx_packet.data_type = 0;

    /* ペイロード長をクリア */
    tx_packet.payload_len = 0;

    /* ペイロードをクリア */
    for(i = 0; i < MAX_PAYLOAD_SIZE; i++)
    {
        tx_packet.payload[i] = 0;
    }

    /* 終了コマンドをクリア */
    tx_packet.data_end_command = 0;    
}


/*=====================================================
 * @brief
 *     受信用パケットを初期化する
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void rx_packet_clear(void)
{
    uint8_t i;

    /* データタイプをクリア */
    rx_packet.data_type = 0;

    /* ペイロード長をクリア */
    rx_packet.payload_len = 0;

    /* ペイロードをクリア */
    for(i = 0; i < MAX_PAYLOAD_SIZE; i++)
    {
        rx_packet.payload[i] = 0;
    }

    /* 終了コマンドをクリア */
    rx_packet.data_end_command = 0;    
}


/******************************************************************************************
 *                                    static関数                                          *
 *****************************************************************************************/

/*-----------------------------------------------------
 * @brief
 *     Slave側の送信準備完了を待つ
 * @param
 *     destination:通信の相手先
 * @return
 *     void:
 * @note
 *     FLAG_CHECK_MODEで通信し、相手側の
 *     tx_packet_statusがTRUEになるまで待つ
 *---------------------------------------------------*/ 
static void flag_check(destination_t destination)
{
    bool_t flag;

    /* flagがTRUEになるまで繰り返す */
    do
    {
        /* プリアンブルを送信 */
        spi_master_send(destination, 0b10101010);
        __delay_ms(10);

        /* Slave命令コマンドを送信 */
        spi_master_send(destination, FLAG_CHECK_MODE);
        __delay_ms(10);

        /* FLAGがTRUEになるまで待つ */
        spi_master_receive(destination, &flag);

        /* 500[ms]インターバルをあける */
        __delay_ms(500);
    } while(flag == FALSE);
}


/*-----------------------------------------------------
 * @brief
 *     uint8_t型のデータをPayloadに格納
 * @param
 *     p_data  :uint8_t型データへのポインタ
 *     data_len:データ個数
 * @return
 *     void:
 * @note
 *     sent_data_set()で空き容量は必ず足りている前提
 *---------------------------------------------------*/
static void uint8_data_set(uint8_t *p_data, uint8_t data_len)
{
    while(data_len)
    {
        tx_packet.payload[tx_packet.payload_len++] = 1;
        tx_packet.payload[tx_packet.payload_len++] = *p_data++;
        data_len--;
    }
}


/*-----------------------------------------------------
 * @brief
 *     uint16_t型のデータをpayloadに格納
 * @param
 *     p_data  :データへのポインタ
 *     data_len:データ数
 * @return
 *     void:
 * @note
 *     sent_data_set()で空き容量は必ず足りている前提
 *---------------------------------------------------*/
static void uint16_data_set(uint16_t *p_data, uint8_t data_len)
{
    uint8_t buf[3];
    uint8_t i;

    while(data_len)                               // data_lenが0になるまで
    {
        uint16_to_byte_array(p_data++, buf);      // payload用のフォーマットに変換する
        
        for(i = 0; i < 3; i++)
        {
            tx_packet.payload[tx_packet.payload_len++] = buf[i]; // payloadに格納する
        }
        data_len--;
    }
}


/*-----------------------------------------------------
 * @brief
 *     double型のデータをpayloadに格納
 * @param
 *     p_data  :データへのポインタ
 *     data_len:データ数
 * @return
 *     void:
 * @note
 *     sent_data_set()で空き容量は必ず足りている前提
 *---------------------------------------------------*/
static void double_data_set(double *p_data, uint8_t data_len)
{
    uint8_t buf[5];
    uint8_t i;

    while(data_len)                               // data_lenが0になるまで
    {
        double_to_byte_array(p_data++, buf);      // payload用フォーマットに変換する

        for(i = 0; i < 5; i++)
        {
            tx_packet.payload[tx_packet.payload_len++] = buf[i]; // payloadに格納する
        }
        data_len--;
    }
}


/*-----------------------------------------------------
 * @brief
 *     uint16_t型のデータを1Byteのデータ配列に変換する
 * @param
 *     p_data:変換したいデータ
 *     p_data_array:変換したいデータを格納する配列(3Byte)
 * @return
 *     void:
 * @note
 *     p_data_arrayには必ず3Byte以上を持たせる
 *---------------------------------------------------*/
static void uint16_to_byte_array(uint16_t *p_data, uint8_t *p_data_array)
{
    const uint16_t bit_mask = 0b0000000011111111;

    /* Payloadフォーマット用に先頭配列に4Byteを格納 */
    p_data_array[0] = 2;
    p_data_array[1] = (uint8_t)(*p_data >> 8);
    p_data_array[2] = (uint8_t)(*p_data & bit_mask);
}


/*-----------------------------------------------------
 * @brief
 *     double型のデータを1byteのデータ配列に変換する
 * @param
 *     p_data      :変換したいデータ
 *     p_data_array:変換したデータを格納する配列(5Byte)
 * @return
 *     void:
 * @note
 *     データ配列は必ず5Byte以上の要素数を持つこと
 *---------------------------------------------------*/
static void double_to_byte_array(double *p_data, uint8_t *p_data_array)
{
    uint8_t i;
    union  // 変換用のunionを定義
    {
        double data;
        uint8_t array[4];      
    } converter;

    /* double型のデータを受け取る */
    converter.data = *p_data;

    /* Payloadフォーマット用に先頭配列に4Byteを格納 */
    p_data_array[0] = 4;
    
    /* 引数で与えられた配列にデータをコピー */
    for(i = 0; i < 4; i++)
    {
        p_data_array[i + 1] = converter.array[i];
    }
}


