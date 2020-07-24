#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include "packet.h"

//#define PACKET_SIZE 100
#define PORT 8882
#define PORT1 8884
#define TIMEOUT 2

char* getTimeStamp(){
    char* arr = (char*) malloc(sizeof(char)*20);
    char milliSec[8];
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);

    struct timeval tv;
    gettimeofday(&tv,NULL);
    strftime(arr, 20, "%H:%M:%S", lt);
    sprintf(milliSec,".%06ld",tv.tv_usec);
    strcat(arr,milliSec);
    return arr;
}


int main(void)
{
	int sd=0,cd=0,master=0,activity;

	struct sockaddr_in addr1,addr2;
	srand(time(0));
	if((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT); // port
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");

    

    if((cd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(PORT1);
    addr2.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(cd , (struct sockaddr*)&addr2, sizeof(addr2) ) == -1)
    {
        perror("bind fail");
        exit(0);
    }
    
    int max = sd;
    if(cd>max)
    	max=cd;

    int slen1 = sizeof(addr1);
    int slen2 = sizeof(addr2);


    PKT rcvpkt,sndpkt,tmp;
    tmp.size = 0;
    tmp.seqno = -1;
    tmp.isLast = 0;
    tmp.isData = 0;

    fd_set readfds;
    while(1)
    {
    	FD_ZERO(&readfds);
        if(cd > 0)   
        FD_SET( cd , &readfds);
        if(sd > 0)
        FD_SET( sd , &readfds);

    	activity = select( max + 1 , &readfds , NULL , NULL , NULL);   

        if ((activity < 0) ) //&& (errno!=EINTR))   
        {   
            printf("select error");   
        }


        if(FD_ISSET(sd , &readfds))
        {
            //rcv and send ack;
            rcvpkt = tmp;
            if(recvfrom(sd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr1, &slen1) == -1)
            {
                printf("recvfrom error..exit\n");
                exit(0);
            }
            printf("Relay1  |  R  |  %s  |  ACK  |  %d  |  server  |  Relay1\n", getTimeStamp(),rcvpkt.seqno);
            // printf("relay 1 got ack %d\n", rcvpkt.seqno);
            // if(rcvpkt.seqno == -1)
            // {
            //     relayOdd = 0;
            // }
            // else
            // {
                /*sndpkt.seqno = rcvpkt.seqno;
                sndpkt.isData = 0;*/
                // sndpkt.


            printf("Relay1  |  S  |  %s  |  ACK  |  %d  |  Relay1  |  CLIENT\n", getTimeStamp(),rcvpkt.seqno);
                if(sendto(cd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr*) &addr2, slen2)==-1)
                {
                  printf("sendto failed exiting...\n");
                  exit(0);
                }
                // printf("relay 1 sent ack %d\n", rcvpkt.seqno);
            // }
        }

        if(FD_ISSET(cd , &readfds))
        {
            //rcv and send ack;
            rcvpkt = tmp;
            if(recvfrom(cd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr2, &slen2) == -1)
            {
                printf("recvfrom error..exit\n");
                exit(0);
            }
            // printf("relay 1 got pkt %d\n", rcvpkt.seqno);
            printf("Relay1  |  R  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay1\n", getTimeStamp(),rcvpkt.seqno);
            // if(rcvpkt.seqno == -1)
            // {
            //     relayEven = 0;
            // }
            // else
            // {
                /*sndpkt.seqno = rcvpkt.seqno;
                sndpkt.isData = 0;*/
                // sndpkt.

            int a = rand();
            // printf("gevd    ::   %d\n", a);
            int dontSend = 0 ;
            a%=100;
            a+=1;
            if(a <= PDR)
            	dontSend = 1;
            if(dontSend == 0)
            {
                a=rand();
                a%=2001;
                usleep(a);
                printf("Relay1  |  S  |  %s  |  DATA  |  %d  |  Relay1  |  SERVER\n", getTimeStamp(),rcvpkt.seqno);
                if(sendto(sd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr*) &addr1, slen1)==-1)
                {
                  printf("sendto failed exiting...\n");
                  exit(0);
                }
                // printf("relay 1 send pkt %d\n", rcvpkt.seqno);
            }
            else
            {
            	printf("Relay1  |  D  |  %s  |  DATA  |  %d  |  Relay1  |  SERVER\n", getTimeStamp(),rcvpkt.seqno);
            	// printf("relay 1 loss!! %d\n", rcvpkt.seqno);
            }
            // }
        }
    }
    

    //now cd gets pkts from clients and sd acks from server...

}

