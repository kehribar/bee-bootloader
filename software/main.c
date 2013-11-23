/*-----------------------------------------------------------------------------
/
/
/
/
/
/
/
/----------------------------------------------------------------------------*/
#include <netdb.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>

#define PAGE_SIZE 128
#define MAXBUF 6553

#include "intel_hex_parser.h"

int yes = 1;
int sock, sockfd, n;
int status, buflen, sinlen;

struct sockaddr_in servaddr,cliaddr;
struct sockaddr_in sock_in;

uint8_t mesg[1000];
uint8_t txBuffer[512];
uint8_t udpBuffer[512];
uint8_t dataBuffer[65536];

int x;
uint32_t xx;
uint8_t qwe = 0;
uint32_t err;
uint32_t count;
float loss;

socklen_t len;	

int sendMessage(uint8_t msgid,uint8_t* msgbuf,uint8_t msglen)
{
	uint8_t i;
	uint32_t r_trial = 0;

	/* magic byte */
	udpBuffer[0] = 0xAA;

	/* message id */
	udpBuffer[1] = msgid ;
	
	/* payload loading */
	for(i=0;i<msglen;i++)
	{
		udpBuffer[i+2] = msgbuf[i];
	}

	buflen = msglen + 2;

	/* send the message */
	sendto(sock, udpBuffer, buflen, 0, (struct sockaddr *)&sock_in, sinlen);

	/* reset the trial counter */
	r_trial = 0;

	/* try to get the ACK */
	while((recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len) <= 0) && (r_trial < 100000))
	{
		r_trial++;
	}

	/* check the result */
	if((mesg[0] != msgid)||(r_trial >= 100000))
	{
		/* error! */
		return -1;
	}
	else
	{
		/* OK! */
		return 1;
	}
}

int main(int argc, char**argv)
{
	int i;
    int fd;
    int offset;
    uint8_t t8;
    int pageNumber;
    int endAddress = 0;
    int startAddress = 1;
    char* fileName = NULL;

	if(argc == 1)
    {
        printf("[err]: What is the file name?\n");        
        return 0;
    }
    else if(argc > 2)
    {
        printf("[err]: Unneccessary parameters!\n");       
        return 0;
    }

	fileName = argv[1];

    memset(dataBuffer, 0xFF, sizeof(dataBuffer));

    parseIntelHex(fileName, dataBuffer, &startAddress, &endAddress);

    if(startAddress != 0)
    {
        printf("[err]: You should change the startAddress = 0 assumption\n");
        return 0;
    }

    if(endAddress > (32768 - 4096))
    {
        printf("[err]: Program size is too big!\n");
        return 0;
    }    

	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	buflen = MAXBUF;

	sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_in.sin_port = htons(0);
	sock_in.sin_family = PF_INET;

	bind(sock, (struct sockaddr *)&sock_in, sinlen);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );

	/* send message to 255.255.255.255 */
	sock_in.sin_addr.s_addr=htonl(-1); 

	/* port number */
	sock_in.sin_port = htons(25600);
	sock_in.sin_family = PF_INET;

	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	memset(&servaddr,0x00,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(32000);
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	
	uint8_t id = 0;
	uint16_t trial;
	uint8_t tmsg = 12;

	printf("[dbg]: Dummy ping to initialize the msgid\n");
	id++;
	trial = 0;
	tmsg = 0;
	while((sendMessage(id,&tmsg,1) < 0) && (trial < 1000))
	{
		trial++;		
	}
	printf("[dbg]: Trial count: %d\n",trial);

	printf("[dbg]: Erasing the memory ...\n");
	id++;
	trial = 0;
	tmsg = 2;
	while((sendMessage(id,&tmsg,1) < 0) && (trial < 1000))
	{
		trial++;		
	}
	printf("[dbg]: Trial count: %d\n",trial);

	i = 0;    
    offset = 0;
    pageNumber = 0;
    while(offset<endAddress)
    {        
        printf("[dbg]: Page number: %d\n",pageNumber);
        printf("[dbg]: Page base address: %d\n",offset);
                       
        for(i=0;i<PAGE_SIZE;i++)
        {                        
            txBuffer[i+1] = dataBuffer[i+offset];
        }

        /* Fill the page buffer command */   
        /* Write the page command */
		id++;
		trial = 0;
		txBuffer[0] = 1;
		txBuffer[129] = (offset >> 0) & 0xFF;
        txBuffer[130] = (offset >> 8) & 0xFF;
        txBuffer[131] = (offset >> 16) & 0xFF;       
        txBuffer[132] = (offset >> 24) & 0xFF;
		while((sendMessage(id,txBuffer,133) < 0) && (trial < 1000))
		{
			trial++;		
		}
		printf("[dbg]: Trial count: %d\n",trial);
        
        offset += PAGE_SIZE;
        pageNumber++;
    }

	/* Jump to user code! */
	id++;
	trial = 0;
	txBuffer[0] = 3;
	while((sendMessage(id,txBuffer,129) < 0) && (trial < 1000))
	{
		trial++;		
	}
	printf("[dbg]: Trial count: %d\n",trial);

	return 0;
}
/*-----------------------------------------------------------------------------------------------*/

