#ifndef _NUMBER_TO_STRING_H_CHAR
#define _NUMBER_TO_STRING_H_CHAR

//#include <stdint.h>


/* RSSI用char型表示用 */
#define ASCII_NUMBER_0 (0x30)
#define MAX_100        (127)
#define MIN_100        (100)
#define MAX_10         (99)
#define MIN_10         (10)
#define MAX_1          (9)
#define MIN_1          (0)
#define CHAR_MINIMUM   (-128)
#define CHAR_MAXIMUM   (127)


/* prototype of function */
void number_to_string_char(char *buf, char num);


#endif
