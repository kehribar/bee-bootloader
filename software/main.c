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
/*---------------------------------------------------------------------------*/
#include "intel_hex_parser.h"
/*---------------------------------------------------------------------------*/
#define PAGE_SIZE 128
#define VERBOSE 0
/*---------------------------------------------------------------------------*/
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
uint8_t qwe;
uint32_t err;
uint32_t count;
float loss;
socklen_t len;	

static void printProgress(float progress);
static void setProgressData(char* friendly, int step);
static int progress_step = 0; // current step
static int progress_total_steps = 0; // total steps for upload
static char* progress_friendly_name; // name of progress section
static int dump_progress = 1; // output computer friendly progress info
static int use_ansi = 1; // output ansi control character stuff

/*-----------------------------------------------------------------------------
/ Send a message over UDP broadcast with a custom packet integrity check.
/------------------------------------------------------------------------------
/ Returns -1 for a failed delivery, 1 for successful delivery.
/------------------------------------------------------------------------------
/ TODO: In some cases, we might have actually delivered the message but  
/ couldn't get the ACK back. We are counting them as a "failure" as well. Try  
/	to find if anything can be done without re-inventing the TCP.
/----------------------------------------------------------------------------*/
int sendMessage(uint8_t msgid,uint8_t* msgbuf,uint8_t msglen);
/*---------------------------------------------------------------------------*/

int main(int argc, char**argv)
{
	int i;
    int offset;    
	uint8_t tmsg;
    int pageNumber;
    uint16_t trial;
    uint8_t id = 0;
    int endAddress = 0;
    int startAddress = 1;
    char* fileName = NULL;

    /*-----------------------------------------------------------------------*/
	
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
	
	/*-----------------------------------------------------------------------*/
	
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

    /*-----------------------------------------------------------------------*/

	sinlen = sizeof(struct sockaddr_in);
	memset(&sock_in, 0, sinlen);
	// buflen = MAXBUF;

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

	/*-----------------------------------------------------------------------*/

	#if VERBOSE
		printf("[dbg]: Dummy ping to initialize the msgid\n");
	#endif

	id++;
	trial = 0;
	tmsg = 0;
	
	while((sendMessage(id,&tmsg,1) < 0) && (trial < 1000))
	{
		trial++;		
	}
	
	#if VERBOSE
		printf("[dbg]: Trial count: %d\n",trial);
	#endif
	
	if(trial >= 1000)
	{
		printf("[err]: Trial threshold is reached\n");
		return 0;
	}

	/*-----------------------------------------------------------------------*/

	printf("> Erasing the memory ...\n");
	
	#if VERBOSE
		printf("[dbg]: Erasing the memory ...\n");	
	#endif

	id++;
	trial = 0;
	tmsg = 2;
	
	while((sendMessage(id,&tmsg,1) < 0) && (trial < 1000))
	{
		trial++;		
	}
	
	#if VERBOSE
		printf("[dbg]: Trial count: %d\n",trial);
	#endif
	
	if(trial >= 1000)
	{
		printf("[err]: Trial threshold is reached\n");
		return 0;
	}

	/*-----------------------------------------------------------------------*/

    offset = 0;    
    pageNumber = 0;

  	setvbuf( stdout, NULL, _IONBF, 0 );

    while(offset<endAddress)
    {        
    	#if VERBOSE
        	printf("[dbg]: Page number: %d\n",pageNumber);
        	printf("[dbg]: Page base address: %d\n",offset);
        #endif
                       
        for(i=0;i<PAGE_SIZE;i++)
        {                        
            txBuffer[i+1] = dataBuffer[i+offset];
        }	

        printf("> Uploading: %3d\r",((100 * offset) / endAddress));

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

		#if VERBOSE
			printf("[dbg]: Trial count: %d\n",trial);
		#endif
		
		if(trial >= 1000)
		{
			printf("[err]: Trial threshold is reached\n");
			return 0;
		}
        
        pageNumber++;
        offset += PAGE_SIZE;        
    }

    /*-----------------------------------------------------------------------*/
	
	printf("\n");
	
	/* Jump to user code! */
	#if VERBOSE
		printf("[dbg]: Jumping to the user code ...\n");	
	#endif

	id++;
	trial = 0;
	txBuffer[0] = 3;

	while((sendMessage(id,txBuffer,1) < 0) && (trial < 1000))
	{
		trial++;
	}

	#if VERBOSE
		printf("[dbg]: Trial count: %d\n",trial);
	#endif

	if(trial >= 1000)
	{
		printf("[err]: Trial threshold is reached\n");
		return 0;
	}

	/*-----------------------------------------------------------------------*/

	return 0;
}
/*-----------------------------------------------------------------------------------------------*/
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
/*-----------------------------------------------------------------------------------------------*/
