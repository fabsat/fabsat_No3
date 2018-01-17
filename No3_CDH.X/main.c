/*
 * 3号機
 * CDH_slave
 */

#include <xc.h>
//#include <math.h>
#include "pic_setting.h"
#include "uart.h"
#include "spi_slave.h"
#include "isr.h"
#include "protocol.h"
#include "AD_value.h"
#include "convert.h"
#include "light.h"
#include "plati_temp.h"

//#include "com_app.h"


/* Configuration bits */
#pragma config FOSC  = HS      // Oscillator Selection bits (HS oscillator)
#pragma config WDTE  = OFF     // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON      // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP   = OFF     // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD   = OFF     // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT   = OFF     // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP    = OFF     // Flash Program Memory Code Protection bit (Code protection off)



/* Prototype of static function */
static void pic_port_init(void);

extern bool_t rx_packet_status;



/******************************************************
 * main function
 *****************************************************/
int main(void)
{    
    uint8_t i;
    uint8_t r_data_type;
    uint8_t r_data[30];
    uint8_t r_data_len;
    
    //uint8_t send_data[] = {'a', 'b', 'c', 'd', 'e'};
    uint8_t send_data[15] = {'a', 'b', 'c', 'd', 'e','f','g','h','i','j','k','l','m'};
     
    uint16_t temp_buf;
    double temp_d;
    uint8_t adcon_shift;
    char temp_c;
    char temp_buf_c[5];
    uint8_t counter;
    double temp, volt; //get_temp()用
    double t_average;
    double light1, light2, light3, light4, light5, light6;
    
    counter = 0;
    
    while(1)
    {
        for(i = 0; i<15;i++)
        {
            send_data[i] = 0;
        }
        pic_port_init();
         
        TRISA = 0b00001111;
        TRISE = 0b00000111;

        /* confirm Power on of PIC */
        RB7 = 1;
        __delay_ms(500);
        RB7 = 0;
        __delay_ms(500);

        /* Initialize UART */
        uart_init();
        spi_slave_start(SPI_ISR_ENABLE);
        
        switch(counter)
        {
            
            case 0:
                //光センサ
        
                send_data[0] = 'L';

                //Light1---------------------------------------
                adcon_shift = 0b10000000;
                volt = get_adcon(adcon_shift);
                light1 = get_pt(volt);
                temp_c = (char)light1;
                number_to_string_char(temp_buf_c, temp_c);
                
                for(i=0; i<5; i++){
                    put_char(temp_buf_c[i]);
                    if(temp_buf_c[i]=='\0'){
                        break;
                    }
                }
                
                    send_data[1] = temp_buf_c[0];
                    send_data[2] = temp_buf_c[1];
                
               
                //LIght2---------------------------------------
                adcon_shift = 0b10001000;
                volt = get_adcon(adcon_shift);
                light2 = get_pt(volt);
                temp_c = (char)light2;
                number_to_string_char(temp_buf_c, temp_c);
                
               
                    send_data[3] = temp_buf_c[0];
                    send_data[4] = temp_buf_c[1];
                
               
                //Light3---------------------------------------
                adcon_shift = 0b10010000;
                volt = get_adcon(adcon_shift);
                light3 = get_pt(volt);
                temp_c = (char)light3;
                number_to_string_char(temp_buf_c, temp_c);
                 
                    send_data[5] = temp_buf_c[0];
                    send_data[6] = temp_buf_c[1];
                
               
                //Light4---------------------------------------
                adcon_shift = 0b10011000;
                volt = get_adcon(adcon_shift);
                light4 = get_pt(volt);
                temp_c = (char)light4;
                number_to_string_char(temp_buf_c, temp_c);
                
                
                    send_data[7] = temp_buf_c[0];
                    send_data[8] = temp_buf_c[1];
                
               
                //Light5--------------------------------------
                adcon_shift = 0b10101000;
                volt = get_adcon(adcon_shift);
                light5 = get_pt(volt);
                temp_c = (char)light5;
                number_to_string_char(temp_buf_c, temp_c);
                
                 
                    send_data[9] = temp_buf_c[0];
                    send_data[10] = temp_buf_c[1];
                
                
                //Light6---------------------------------------
                adcon_shift = 0b10110000;
                volt = get_adcon(adcon_shift);
                light6 = get_pt(volt);
                temp_c = (char)light6;
                number_to_string_char(temp_buf_c, temp_c);
                
                
                    send_data[11] = temp_buf_c[0];
                    send_data[12] = temp_buf_c[1];
                
                
                send_data[13] = '\0';
                break;
            
            
            case 1:
                //電源
                //t_average = 0.0;
                
                send_data[0] = 'V';
                adcon_shift = 0b10111001;
                /*
                for(i = 0; i <= 10; i++){
                    volt = get_adcon(adcon_shift);
                    temp = get_temp(volt);
                    t_average += temp;
                }
                t_average /= 10.0;
               
                //printf("temp = %f\r\n",temp);
                temp_c = (char)t_average;
                */
                volt = get_adcon(adcon_shift);
                temp = volt * 10.0;
                temp_c = (char)temp;
                number_to_string_char(temp_buf_c, temp_c);
                
                
                    send_data[1] = temp_buf_c[0];
                    send_data[2] = temp_buf_c[1];
                
                
                
                send_data[3] = '\0';
                break;
                
               
        }
        
        //姿勢と電源の切り替え
        if(counter == 0){
            counter = 1;
        }
        else{
            counter = 0;
        }
        put_string(send_data);
        put_char('\r');
        put_char('\n');
        
        
        /* Start message transmited */
        put_string("Slave Program Start!\r\n");


        RB7 = 1;
        __delay_ms(100);
        RB7 = 0;
        __delay_ms(100);

         while(rx_packet_status == FALSE);

        extract_receive_packet(&r_data_type, r_data, &r_data_len);
        put_char(r_data[0]);
        put_char(r_data[1]);

        if(r_data[1] == 0x02)           // 0x01 -> RX_MODE
        {
            __delay_ms(500);   //5000

            /* 送信のとき */

            sent_data_set(send_data, sizeof(send_data), sizeof(uint8_t));
            send_data_slave(DATA, DATA_END);

        }
    }
    
    return 0;
}


/*-----------------------------------------------------
 * @brief
 *     Set all port OUTPUT
 * @param
 *     none:
 * @return
 *     none:
 * @note
 *     Prevent for PORT incorrect action
 *---------------------------------------------------*/
static void pic_port_init(void)
{
    /* Set all port OUTPUT */
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    
    /* All port are driven low */
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
}

