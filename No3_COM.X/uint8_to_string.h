#ifndef _UINT8_T_STRING
#define _UINT8_T_STRING


#include <xc.h>
#include "pic_setting.h"


/* Prototype of Function */
/*=====================================================
 * @brief
 *     UINT8_T number to String
 * @param
 *     num      :number to want to change
 *     p_str_buf:buffer for String filled in
 *     buf_size :buffer size(recommend to use sizeof)
 * @return
 *     none:
 * @note
 *     uint8_t (0 ~ 255)
 *     buffer need more than 4
 *===================================================*/
void uint8_to_string(uint8_t num, uint8_t *p_str_buf, uint8_t buf_size);


#endif
