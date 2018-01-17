
#include <xc.h>
#include "uint8_to_string.h"
#include "pic_setting.h"


/* Define List */
#define ASCII_NUMBER_0 0x30
#define MAX_100        255
#define MIN_100        100
#define MAX_10         99
#define MIN_10         10
#define MAX_1          9
#define MIN_1          0


/* Prototype of Function */
static void process_100(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag);
static void process_10(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag);
static void process_1(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag);



/*=====================================================
 * @brief
 *     UINT8_T number to String
 * @param
 *     num      :numbert to want to change
 *     p_str_buf:buffer for String filled in
 *     buf_size :buffer size(recommend to use sizeof)
 * @return
 *     none:
 * @note
 *     uint8_t (0 ~ 255)
 *     buffer need more than 4
 *===================================================*/
void uint8_to_string(uint8_t num, uint8_t *p_str_buf, uint8_t buf_size)
{
    uint8_t flag = 0;

    /* String Buffer size is shortage */
    if(buf_size < 4)
    {
        return;
    }

    /* uint8_t Number to String */
    if(MIN_100 <= num && num <= MAX_100)
    {
        process_100(num, p_str_buf, &flag);
    }
    else if(MIN_10 <= num && num <= MAX_10)
    {
        process_10(num, p_str_buf, &flag);
    }
    else if(MIN_1 <= num && num <= MAX_1)
    {
        process_1(num, p_str_buf, &flag);
    }
}


/*-----------------------------------------------------
 * PROCESS 100
 *---------------------------------------------------*/
static void process_100(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag)
{
    /* Judge 100 or 200 */
    if(MIN_1 <= num && num <= 199)
    {        
        p_str_buf[*p_flag] = ASCII_NUMBER_0 + 1;
        (*p_flag)++;
    }
    else if(200 <= num && num <= MAX_100)
    {
        p_str_buf[*p_flag] = ASCII_NUMBER_0 + 2;
        (*p_flag)++;
    }

    /* Be changed to PROCESS 10 */
    process_10((num % 100), p_str_buf, p_flag);
}


/*-----------------------------------------------------
 * PROCESS 10
 *---------------------------------------------------*/
static void process_10(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag)
{
    uint8_t divide = num / 10;
    uint8_t val = ASCII_NUMBER_0;
    uint8_t i;


    for(i = 0; i < divide; i++)
    {
        val++;
    }

    p_str_buf[*p_flag] = val;
    (*p_flag)++;

    process_1((num % 10), p_str_buf, p_flag);
}


/*-----------------------------------------------------
 * PROCESS 1
 *---------------------------------------------------*/
static void process_1(uint8_t num, uint8_t *p_str_buf, uint8_t *p_flag)
{
    uint8_t i;
    uint8_t val = ASCII_NUMBER_0;

    for(i = 0; i < num; i++)
    { 
        val++;
    }

    p_str_buf[*p_flag] = val;
    (*p_flag)++;

    p_str_buf[*p_flag] = '\0';
}


