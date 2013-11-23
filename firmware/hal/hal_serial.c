/*-----------------------------------------------------------------------------
/ 
/	Serial port related functions.
/
/------------------------------------------------------------------------------
/   ihsan Kehribar - 2013 
/----------------------------------------------------------------------------*/
#include "hal_serial.h"
/*---------------------------------------------------------------------------*/
void init_serial()
{
    /* 38400 baud rate with 8 MHz F_OSC ... */
    const uint8_t ubrr = 12;

    pinMode(D,0,INPUT);
    pinMode(D,1,OUTPUT);

    /* Set baud rate */ 
    UBRR0H = (unsigned char)(ubrr>>8); 
    UBRR0L = (unsigned char)ubrr; 

    /* Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    /* Set frame format: 8data, 1stop bit no parity */ 
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); 

    /* Set the library ... */
    xfunc_out = send_char;
}
/*---------------------------------------------------------------------------*/
void send_char(char c)
{
    /* Wait for empty transmit buffer ... */
    while(!(UCSR0A & (1<<UDRE0)));

    /* Start sending the data! */
    UDR0 = c;

    /* Wait until the transmission is over ... */
    while(!(UCSR0A & (1<<TXC0)));
}
/*---------------------------------------------------------------------------*/
