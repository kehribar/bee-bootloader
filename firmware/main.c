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
#define MYUDPPORT 32000
/*---------------------------------------------------------------------------*/
uint8_t pageBuf[128];
void (*funcptr)(void) = 0x0000;
/*---------------------------------------------------------------------------*/
static uint8_t buf[BUFFER_SIZE+1];
static const uint16_t broadcastport = 32000;
static const uint8_t myip[4] = {255,255,255,255};
static const uint8_t broadcastip[4] = {255,255,255,255};
static const uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x29};
static const uint8_t broadcastmac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};    
/*---------------------------------------------------------------------------*/
#define check_new_UDP_message() ((plen>0) && (buf[IP_PROTO_P]==IP_PROTO_UDP_V))
#define message_length_UDP() (((buf[UDP_LEN_H_P]<<8) + buf[UDP_LEN_L_P]) - UDP_HEADER_LEN)
/*---------------------------------------------------------------------------*/
static void sendBroadcast(uint8_t* mbuf, uint8_t len)
{
    uint8_t xc = 0;

    send_udp_prepare(buf,broadcastport,broadcastip,broadcastport,broadcastmac);
    while(xc < len)
    {
        buf[UDP_DATA_P+xc] = mbuf[xc];
        xc++;
    }
    send_udp_transmit(buf,len);
}
/*---------------------------------------------------------------------------*/
static void boot_program_page(uint32_t page, uint8_t *buf)
{
    uint16_t i;

    cli();
    eeprom_busy_wait();

    for(i=0;i<SPM_PAGESIZE;i+=2)
    {
        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill(page+i,w);
    }

    boot_page_write(page);
    boot_spm_busy_wait();  

    boot_rww_enable();
}
/*---------------------------------------------------------------------------*/
int main(void)
{   
    uint8_t t8;
    uint32_t x;    
    uint16_t qw;    
    uint32_t t32;
    uint8_t prev;
    uint8_t curr;
    uint8_t* pbuf;
    uint16_t plen;
    uint32_t pageNum;
    uint16_t timeout;
    uint8_t first = 1;    
    uint8_t boot_state; 
    uint8_t isDuplicate;
    uint8_t* udp_data = buf + UDP_DATA_P;

    wdt_reset();
    wdt_enable(WDTO_8S);    

    init_serial();
    init_mac(mymac);      
    init_udp_or_www_server(mymac,myip);             

    enc28j60Init(mymac);        
    enc28j60PhyWrite(PHLCON,0x476);
    enc28j60EnableBroadcast();

    /* main loop */
    while(1)
    {    
        /* kick the dog ... */
        wdt_reset();

        /* poll hardware ethernet buffer */
        plen = enc28j60PacketReceive(BUFFER_SIZE,buf);

        /*---------------------------------------------------------------------
        / Packet structure:
        /   [0]: Fixed value, 0xAA
        /   [1]: Message ID
        /   [2]: Command type
        /   [3]: Payload start position
        /--------------------------------------------------------------------*/
        if(check_new_UDP_message() && (udp_data[0] == 0xAA))
        {
            /* Reset the timeout */
            timeout = 0;
            
            /* Parse the message id and send it back as an ack */                      
            t8 = udp_data[1];            
            sendBroadcast(&t8,1);            

            prev = curr;
            curr = udp_data[1];
                        
            isDuplicate = 0;            
            
            /* Check the message ID */
            if(prev == curr)
            {
                if(!first)
                { 
                    isDuplicate = 1;
                }
                else
                {
                    first = 0;               
                }
            }
            else if((curr - prev) != 1)
            {
                if(!((curr == 0) && (prev == 255)))
                {
                    if(!first)
                    {
                        /* TODO: Recover from here ... */
                        xprintf(PSTR("Fatal error!\r\n"));
                        xprintf(PSTR("p: %3u  c: %3u\r\n"),prev,curr);
                        while(1);                
                    }
                    else
                    {
                        first = 0;
                    }                    
                }                
            }            

            /* If the message isn't a duplicate, take action ... */
            if(!isDuplicate)
            {                
                /* bootloader state machine */
                switch(udp_data[2])
                {
                    /* ping request */
                    case 0:
                    {                
                        /* empty ... */
                        break;
                    }
                    /* load page buffer and initiate write */
                    case 1:
                    {
                        for(t8=0;t8<128;t8++)
                        {
                            pageBuf[t8] = udp_data[3+t8];
                        }                                                

                        pageNum = udp_data[131];
                
                        t32 = udp_data[132];
                        t32 = t32 << 8;
                        pageNum += t32;

                        t32 = udp_data[133];
                        t32 = t32 << 16;
                        pageNum += t32;

                        t32 = udp_data[134];
                        t32 = t32 << 24;
                        pageNum += t32;

                        boot_program_page(pageNum,pageBuf);                       
                        break;
                    }                    
                    /* delete user application */
                    case 2:
                    {
                        for(t32=0;t32<0x7000;t32+=SPM_PAGESIZE)
                        {                 
                            boot_page_erase(t32);
                            boot_spm_busy_wait();
                        }
                        break;
                    }
                    /* jump to the user application */
                    case 3:
                    {
                        /* Make sure the software gets the ACK */
                        for (t8 = 0; t8 < 32; ++t8)
                        {                            
                            sendBroadcast(udp_data,1);
                            _delay_ms(1);
                        }             
                        MCUSR &= ~(1 << WDRF);
                        wdt_disable();
                        funcptr();
                        break;
                    }                    
                }
            }
        }    
        else if(timeout++ > 0xFFFE)
        {            
            /* TODO: Add valid user code detection here */
            timeout = 0;
            MCUSR &= ~(1 << WDRF);
            wdt_disable();
            funcptr();
        }          
    }    

    return 0;
}
/*---------------------------------------------------------------------------*/