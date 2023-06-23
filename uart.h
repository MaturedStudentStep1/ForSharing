/*
 * uart.h
 *
 *  Created on  : May 30, 2021
 *  Created by  : Tommy Thai
 *  Modified on :
 *  Modified by :
 */

#ifndef UART_H_
#define UART_H_

#include <msp430.h>
#include <string.h>

#define UART_01MHz_9600     1
#define UART_01MHz_115200   2
#define UART_08MHz_9600     3
#define UART_08MHz_115200   4
#define UART_16MHz_9600     5
#define UART_16MHz_115200   6

void uart_init(unsigned char MHz_BaudRate);
char uart_data_ready();
unsigned char uart_getc();
void uart_gets(char *s);

void uart_send_char( unsigned char data );
void uart_send_string(unsigned char *str);
void uart_send_str_len(unsigned char *str, unsigned char len);
void uart_send_int(int data);
void uart_send_int32(long long data);
void uart_send_float(float data);
void uart_send_char_hex(unsigned char val);

#endif /* UART_H_ */
