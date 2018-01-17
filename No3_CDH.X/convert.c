//#include <stdint.h>
//#include <stdio.h>
#include "convert.h"


/* Type Define */
typedef unsigned char uint8_t;


/* Prototype of function */
static void process_100(char num, uint8_t *p_flag, char *p_buf);
static void process_10(char num, uint8_t *p_flag, char *p_buf);
static void process_1(char num, uint8_t *p_flag, char *p_buf);
static void process_1_null(char num, uint8_t *p_flag, char *p_buf);


/*=====================================================
 * char用数値<=>文字列変換
 * char (-128 ~ 127)
 * bufは5以上の配列
 *===================================================*/
void number_to_string_char(char *buf, char num)
{
    uint8_t flag = 0;

    /* num == -128 process */
    if(num == CHAR_MINIMUM)
    {
        buf[0] = '-';
        buf[1] = '1';
        buf[2] = '2';
        buf[3] = '8';
        buf[4] = '\0';
        
        return;
    }

    /* For negative number process */
    if(num < 0)
    {
        buf[flag] = '-';
        flag++;
        num *= (-1);     // 128はcharでは受け取れないので-128はダメ
    }

    /* converting process */
    if(MIN_100 <= num && num <= MAX_100)
    {
        process_100(num, &flag, buf);
    }
    else if(MIN_10 <= num && num <= MAX_10)
    {
        process_10(num, &flag, buf);
    }
    else if(MIN_1 <= num && num <= MAX_1)
    {
        //process_1(num, &flag, buf);
        //1桁の時は0を含める
        process_1_null(num, &flag, buf);
    }
    else
    {
        //printf("number size is over\n");
    }
}


/*-----------------------------------------------------
 * 100の位を文字へ
 *---------------------------------------------------*/
static void process_100(char num, uint8_t *p_flag, char *p_buf)
{
    uint8_t remainder_100 = num % 100;
    
    p_buf[*p_flag] = ASCII_NUMBER_0 + 1;
    (*p_flag)++;
    
    process_10(remainder_100, p_flag, p_buf);
}


/*-----------------------------------------------------
 * 10の位を文字へ
 *---------------------------------------------------*/
static void process_10(char num, uint8_t *p_flag, char *p_buf)
{
    uint8_t divide_10    = num / 10;
    uint8_t remainder_10 = num % 10;
    uint8_t val          = ASCII_NUMBER_0;
    uint8_t i;
    
    for(i = 0; i < divide_10; i++)
    {
        val++;
    }
    
    p_buf[*p_flag] = val;
    (*p_flag)++;
    
    process_1(remainder_10, p_flag, p_buf);
}


/*-----------------------------------------------------
 * 1の位を文字へ
 *---------------------------------------------------*/
static void process_1(char num, uint8_t *p_flag, char *p_buf)
{
    uint8_t divide_1 = num;
    uint8_t val      = ASCII_NUMBER_0;
    int i;
    
    for(i = 0; i < divide_1; i++)
    {
        val++;
    }
    
    p_buf[*p_flag] = val;
    (*p_flag)++;
    
    p_buf[*p_flag] = '\0';
}
/*-----------------------------------------------------
 * 1の位を文字へ 0考慮版
 *---------------------------------------------------*/
static void process_1_null(char num, uint8_t *p_flag, char *p_buf)
{
    uint8_t divide_1 = num;
    uint8_t val      = ASCII_NUMBER_0;
    int i;
    
    p_buf[*p_flag] = ASCII_NUMBER_0;
    (*p_flag)++;
    
    for(i = 0; i < divide_1; i++)
    {
        val++;
    }
    
    p_buf[*p_flag] = val;
    (*p_flag)++;
    
    p_buf[*p_flag] = '\0';
}

