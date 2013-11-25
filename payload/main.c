/*-----------------------------------------------------------------------------
/
/
/
/
/
/
/----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
/*---------------------------------------------------------------------------*/
#include "./hal/hal.h"
#include "./tuxlib/tuxlib.h"
/*---------------------------------------------------------------------------*/
#define BUFFER_SIZE 750
/*---------------------------------------------------------------------------*/
void checkBootloaderCondition(uint8_t* ethBuf);
/*---------------------------------------------------------------------------*/
static uint8_t buf[BUFFER_SIZE+1];
static const uint8_t myip[4] = {192,168,1,125};
static const uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x29};
/*---------------------------------------------------------------------------*/
int main(void)
{
    uint16_t plen;    
    
    init_serial();
    init_mac(mymac);      
    init_udp_or_www_server(mymac,myip);             

    enc28j60Init(mymac);        
    enc28j60PhyWrite(PHLCON,0x476);
    enc28j60EnableBroadcast();

    xprintf(PSTR("Hello World!\r\n"));

    /* main loop */
    while(1)
    {            
        /* poll hardware ethernet buffer */
        plen = enc28j60PacketReceive(BUFFER_SIZE,buf);

        /* any new message? */
        if(plen > 0)
        {
            checkBootloaderCondition(buf);
        }

    }
    return 0;
}
/*---------------------------------------------------------------------------*/
void checkBootloaderCondition(uint8_t* ethBuf)
{
    uint16_t t16;
    void (*bootloaderPointer)(void) = 0x7000;

    /* message is UDP type */
    if((ethBuf[IP_PROTO_P]==IP_PROTO_UDP_V))
    {                
        /* calculate the UDP port */
        t16 = (ethBuf[UDP_DST_PORT_H_P] << 8) + ethBuf[UDP_DST_PORT_L_P];

        /* check the port */
        if(t16 == 55555)
        {  
            /* check the magic key */
            if((ethBuf[UDP_DATA_P]==0x55) && (ethBuf[UDP_DATA_P+1]==0xAA))
            {
                /* jump to the bootloader! */
                bootloaderPointer();
            }
        }
    }

}
/*---------------------------------------------------------------------------*/