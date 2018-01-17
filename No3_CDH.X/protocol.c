/******************************************************************************************
 * protocol.c
 * ver1.00
 * Hirofumi Hamada
 *=========================================================================================
 * 通信プロトコルについてのライブラリ　ソースファル(COM)
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
#include "spi_slave.h"
//#include "uart.h"
//#include "uint8_to_string.h"


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
static packet_format_t rx_packet;  // RX:データがextractされたら初期化される;

/* tx_packetが送信準備完了かどうかを表す(TRUE：送信準備完了 FALSE：パケット準備中) */
bool_t tx_packet_status = FALSE;

/* rx_packetの状態を表す変数(TRUE:パケットFULL FALSE:パケットEmpty ) */
bool_t rx_packet_status = FALSE;



/******************************************************************************************
 *                                static関数のプロトタイプ宣言                              *
 *****************************************************************************************/
static void uint8_data_set(uint8_t *p_data, uint8_t data_len);
static void uint16_data_set(uint16_t *p_data, uint8_t data_len);
static void double_data_set(double *p_data, uint8_t data_len);
static void uint16_to_byte_array(uint16_t *p_data, uint8_t *p_data_array);
static void double_to_byte_array(double *p_data, uint8_t *p_data_array);
static void buf_send(uint8_t *buf);




/******************************************************************************************
 *                                   ユーザーライブラリ関数                                 *
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
 *     パケットの送信準備完了を告げる
 *     同時にデータタイプと終了コマンドを渡す
 * @param
 *     
 * @return
 *     void:
 * @note
 *     最後にtx_packet_statusをTRUEとして、
 *     Masterからの送信命令を待つ
 *===================================================*/
void send_data_slave(data_type_t data_type, data_end_command_t data_end_command)
{
    /* データタイプと終了コマンドをパケットに格納 */
    tx_packet.data_type        = (uint8_t)data_type;
    tx_packet.data_end_command = (uint8_t)data_end_command;

    /* 送信準備完了としてstatusをTRUEに変更する */
    tx_packet_status = TRUE;
}


/*=====================================================
 * @brief
 *     受信パケットからデータを取り出す
 * @param
 *     p_data_type_buf:データタイプを取り出す用のポインタ
 *     p_data_buf     :ペイロードを取り出す用のポインタ
 *                     30byte以上必要
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

    /* rx_packet_statusをFALSEへ */
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
 *                               システム内ライブラリ関数                                 *
 *****************************************************************************************/

/*=====================================================
 * @brief
 *     パケットを送信する
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void send_packet(void)
{
    uint8_t buf;
    uint8_t i;
    
    /* データタイプを送信する */
    spi_slave_send(tx_packet.data_type);

    /* ペイロード長を送信する */
    spi_slave_send(tx_packet.payload_len);

    /* ペイロードを送信する */
    for(i = 0; i < tx_packet.payload_len; i++)
    {
        spi_slave_send(tx_packet.payload[i]);
    }

    /* 終了コマンドを送信する */
    spi_slave_send(tx_packet.data_end_command);
    
    /* tx_packetを初期化する */
    tx_packet_clear();

    /* tx_packet_statusをFALSEに戻す */
    tx_packet_status = FALSE;
}


/*=====================================================
 * @brief
 *     パケットを受信する(RX_MODE)
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     受信後rx_packetにデータを移す
 *     受信後rx_packet_statusをTRUEに変更する
 *===================================================*/
void receive_packet(void)
{
    uint8_t receive_buf[33];
    uint8_t i;
    
    /* データタイプとペイロード長を受け取る */
    for(i = 0; i < 2; i++)
    {
        spi_slave_receive(&receive_buf[i]);
    }

    /* ペイロードと終了コマンドを受け取る(ペイロード長で条件式を作る) */
    for(; i <= (receive_buf[1] + 2); i++)
    {
        spi_slave_receive(&receive_buf[i]);
    }

    /* バッファからrx_packetにデータを移す */
    buf_send(receive_buf);

    /* 受信パケットにデータありのフラグを立てる */
    rx_packet_status = TRUE;
}


/*=====================================================
 * @brief
 *     tx_packet_statusの状態に応じて、
 *     Master側にフラグを渡す(FLAG_CHECK_MODE)
 * @param
 *     void:
 * @return
 *     void:
 * @note
 *     note
 *===================================================*/
void check_flag(void)
{
    /* tx_packetの準備状態をSPIバッファにセット */
    if(tx_packet_status)
    {
        spi_slave_send((uint8_t)TRUE);
    }
    else
    {
        spi_slave_send((uint8_t)FALSE);
    }
}


/******************************************************************************************
 *                                    static関数                                          *
 *****************************************************************************************/

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


/*-----------------------------------------------------
 * @brief
 *     受信したパケットをrx_packetにコピーする
 * @param
 *     buf:受信データが入っているバッファ
 * @return
 *     void:
 * @note
 *     note
 *---------------------------------------------------*/
static void buf_send(uint8_t *buf)
{
    uint8_t i;

    /* データタイプを受け取る */
    rx_packet.data_type = *buf++;
    
    /* ペイロード長を受け取る */
    rx_packet.payload_len = *buf++;

    /* ペイロードを受け取る */
    for(i = 0; i < rx_packet.payload_len; i++)
    {
        rx_packet.payload[i] = *buf++;
    }

    /* 終了コマンドを受け取る */
    rx_packet.data_end_command = *buf;
}


