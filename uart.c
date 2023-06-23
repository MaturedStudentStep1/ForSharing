/*
 * uart.c
 *
 *  Created on  : July 02, 2021
 *  Created by  : Tommy Thai
 *  Modified on :
 *  Modified by :
 */

#include "uart.h"

void uart_init(unsigned char MHz_BaudRate)
{

    // (1) Set state machine to the reset state
    UCA0CTL1 = UCSWRST;

    // (2) Initialize USCI registers
    UCA0CTL0 |= 0x00;
    UCA0CTL1 |= UCSSEL_2;          // CLK = SMCLK

    switch(MHz_BaudRate)
    {
        case UART_01MHz_9600 :
            UCA0MCTL = UCBRS0;
            UCA0BR0 = 104;
            UCA0BR1 = 0;
            break;
        case UART_01MHz_115200 :
            UCA0MCTL = UCBRS2 + UCBRS0;
            UCA0BR0 = 8;
            UCA0BR1 = 0;
            break;
        case UART_08MHz_9600 :
            UCA0MCTL  = UCBRF_1 |  UCBRS_0 | UCOS16 ;
            UCA0BR0 = 52;
            UCA0BR1 = 00;
            break;
        case UART_08MHz_115200 :
            UCA0MCTL  = UCBRS_5 |  UCBRF_3 | UCOS16 ;
            UCA0BR0 = 4;
            UCA0BR1 = 0;
            break;
        case UART_16MHz_9600 :
            UCA0BR0 = 130;
            UCA0BR1 = 6;
            break;
        case UART_16MHz_115200 :
            UCA0MCTL = UCBRS2 +UCBRS1+ UCBRS0;
            UCA0BR0 = 138;
            UCA0BR1 = 0;
    }


    // (3) Configure ports
    P1SEL = BIT1 + BIT2;                // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;                // P1.1 = RXD, P1.2=TXD

    // (4) Clear UCSWRST flag
    UCA0CTL1 &= ~UCSWRST;               // **Initialize USCI state machine**

    // (5) Enable USCI_A0 RX interrupt
    IE2 |= UCA0RXIE;

}
char uart_data_ready()
{
    if (UCA0RXIFG)
        return 1;
    else
        return 0;

}
unsigned char uart_getc()
{
    unsigned char timeout = 0;

    while (!(IFG2 & UCA0RXIFG)) // Wait until USCI_A0 RX receive a complete character
    {
        timeout++;
        if (timeout < 1000)
            break;
    }
    if (timeout < 999)
        return UCA0RXBUF;
    else
        return 0;
}
void uart_gets(char *s)
{
    *s = uart_getc();
    while (*s != '\0')
    {
        s++;
        *s = uart_getc();
    }
}
void uart_send_char(unsigned char data)
{
    // Wait until USCI_A0 TX buffer is ready
    while (!(IFG2 & UCA0TXIFG))
        ;

    // Send the byte
    UCA0TXBUF = data;

}
void uart_send_string(unsigned char *str)
{
    unsigned int i;
    unsigned int len;
    len = strlen((const char*) str);
    for (i = 0; i < len; i++)
    {
        uart_send_char(str[i]);
    }
}
void uart_send_str_len(unsigned char *str, unsigned char len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        uart_send_char(str[i]);
    }
}
void uart_send_int(int data)
{
    unsigned char i;
    unsigned char *zPoint;
    zPoint = (unsigned char*)&data;
    for(i=0;i<2;i++)
    {
        // Wait until USCI_A0 TX buffer is ready
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = zPoint[i];
    }
}
void uart_send_int32(long long data)
{
    unsigned char i;
    unsigned char *zPoint;
    zPoint = (unsigned char*)&data;

    for(i=0;i<4;i++)
    {
          // Wait until USCI_A0 TX buffer is ready
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = zPoint[i];
    }

}
void uart_send_float(float data)
{
    unsigned char i;
    unsigned char *zPoint;
    zPoint = (unsigned char*)&data;

    for(i=0;i<4;i++)
    {
          // Wait until USCI_A0 TX buffer is ready
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = zPoint[i];
    }

}

void uart_send_char_hex(unsigned char val)
{
    unsigned char output[3];
    int i, remainder;
    char digitsArray[17] = "0123456789ABCDEF";

    for (i = 2; i > 0; i--)
    {
        remainder = val % 16;
        val = val / 16;
        output[i - 1] = digitsArray[remainder];
    }
    output[2] = '\0';
    uart_send_string(output);
}
