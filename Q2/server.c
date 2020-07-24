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

#define PORT 8882
#define PORT1 8883  


void shiftbuff(PKT copy[],int shift)
{
    int i;
    for(i=0;i + shift < WINDOW;i++)
    {
        copy[i].seqno = copy[i+shift].seqno;
        copy[i].isLast = copy[i+shift].isLast;
        copy[i].isData = copy[i+shift].isData;
        copy[i].size = copy[i+shift].size;
        strcpy(copy[i].payload , copy[i+shift].payload);
    }
    for(;i<WINDOW;i++)
    {
        copy[i].seqno = -1;
        copy[i].isLast = 0;
        copy[i].isData = 1;
        copy[i].size = 0;
        
    }
}

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
    int relayEven,relayOdd,master,opt=1,activity;
    relayEven=relayOdd=0;
    struct sockaddr_in addr,addr1; 
    if( (relayOdd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }

    addr.sin_family = AF_INET;   
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   
    addr.sin_port = htons( PORT );   
         

    if( (relayEven = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }

    addr1.sin_family = AF_INET;   
    addr1.sin_addr.s_addr = htonl(INADDR_ANY);   
    addr1.sin_port = htons( PORT1 );  
    //bind the socket to localhost port 8888
    
    if( bind(relayOdd , (struct sockaddr*)&addr, sizeof(addr) ) == -1)
    {
        perror("bind fail");
        exit(0);
    }
    
    if( bind(relayEven , (struct sockaddr*)&addr1, sizeof(addr1) ) == -1)
    {
        perror("bind fail");
        exit(0);
    }
    // printf("Bind Done\n");

    fd_set readfds; 
    int max = relayOdd;
    if(relayEven>max)
        max = relayEven;


    PKT rcvpkt,sndpkt,tmp;
    tmp.size = 0;
    tmp.seqno = -1;
    tmp.isLast = 0;
    tmp.isData = 0;
    int slen = sizeof(addr);
    int slen1 = sizeof(addr1);

    FILE *fp = fopen("destination.txt","wb"); // wb nhi krna haina?? puch le.
    if(fp==NULL)
    {
        printf("File open error");
        return 1;   
    }
    int i;
    PKT buffer[WINDOW+1];
    for(i=0;i<WINDOW;i++)
    {
        buffer[i].seqno = -1;
    }
    int expt = 0;
    int p1 = 0,p2 = 0;
    int last = 0;   
    struct timeval t;
    t.tv_sec = 6;
    t.tv_usec = 0;
    while( (p1==0||p2==0) && last == 0)//yaha dekh lio gadbad..
    {
        FD_ZERO(&readfds);
//        if(relayEven > 0)   
        FD_SET( relayEven , &readfds);
//        if(relayOdd > 0)
        FD_SET( relayOdd , &readfds);
        // printf("before act %d  : %d\n",buffer[0].seqno,expt);
        activity = select( max + 1 , &readfds , NULL , NULL ,NULL);   
        // printf("after act %d\n",last);
        if ((activity < 0) ) //&& (errno!=EINTR))   
        {   
            printf("select error");   
        }

        if(FD_ISSET(relayOdd , &readfds))
        {
            //rcv and send ack;
            rcvpkt = tmp;
            // printf(" relayOdd %d %d\n",relayOdd,slen);
            if(recvfrom(relayOdd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr, &slen) == -1)
            {
                printf("recvfrom error..exit\n");
                exit(0);
            }
            printf("SERVER  |  R  |  %s  |  DATA  |  %d  |  Relay1  |  SERVER\n", getTimeStamp(),rcvpkt.seqno);
            // printf("GOT PKT : %d\n", rcvpkt.seqno);
            if(rcvpkt.seqno == -1)
            {
                printf("ERROR IN RECEIVE\n");
                p1 = 1;
            }
            else
            {
                if(rcvpkt.seqno == expt)
                {
                    fwrite(rcvpkt.payload, 1 , rcvpkt.size , fp);
                    // printf("%d writing to file : %s\n",rcvpkt.seqno, rcvpkt.payload);
                    if(rcvpkt.isLast==1)
                        last=1;
                    int shift = 1;
                    for(i=1;i<WINDOW;i++)
                    {
                        if(buffer[i].seqno != -1)
                        {
                            fwrite(buffer[i].payload, 1 , buffer[i].size , fp);
                            // printf("%d writing to file : %s\n",buffer[i].seqno, buffer[i].payload);
                            if(buffer[i].isLast==1)
                                last=1; 
                            shift++ ;
                        }
                        else break;
                    }
                    expt +=shift;
                    shiftbuff(buffer,shift);

                }
                else
                {
                    buffer[rcvpkt.seqno - expt].seqno = rcvpkt.seqno;
                    buffer[rcvpkt.seqno - expt].size = rcvpkt.size;
                    buffer[rcvpkt.seqno - expt].isLast = rcvpkt.isLast;
                    buffer[rcvpkt.seqno - expt].isData = rcvpkt.isData;
                    strcpy(buffer[rcvpkt.seqno - expt].payload,rcvpkt.payload);
                }
                // printf("data : %s\n", rcvpkt.payload);
                
                // else
                // {
                    sndpkt.seqno = rcvpkt.seqno;
                    sndpkt.isData = 0;
                    // sndpkt.
                // printf("SENT ACK : %d\n", rcvpkt.seqno);
                    printf("SERVER  |  S  |  %s  |  ACK  |  %d  |  SERVER  |  Relay1\n", getTimeStamp(),rcvpkt.seqno);
                    if(sendto(relayOdd, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr, slen)==-1)
                    {
                      printf("sendto failed exiting...\n");
                      exit(0);
                    }
            }
        }

        if(FD_ISSET(relayEven , &readfds))
        {
            //rcv and send ack;
            rcvpkt = tmp;
            if(recvfrom(relayEven, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr1, &slen1) == -1)
            {
                printf("recvfrom error..exit\n");
                exit(0);
            }
            printf("SERVER  |  R  |  %s  |  DATA  |  %d  |  Relay2  |  SERVER\n", getTimeStamp(),rcvpkt.seqno);
            // printf("GOT PKT : %d  :  %d\n", rcvpkt.seqno , rcvpkt.size);
            if(rcvpkt.seqno == -1)
            {
                printf("ERROR WHILE RECEIVE\n");
                p2 = 1;
            }
            else
            {
                if(rcvpkt.seqno == expt)
                {
                    fwrite(rcvpkt.payload, 1 , rcvpkt.size , fp);
                    if(rcvpkt.isLast==1)
                        last=1;
                    // printf("%d writing to file : %s\n",rcvpkt.seqno, rcvpkt.payload);
                    int shift = 1;
                    for(i=1;i<WINDOW;i++)
                    {
                        if(buffer[i].seqno != -1)
                        {
                            fwrite(buffer[i].payload, 1 , buffer[i].size , fp);
                            // printf("%d writing to file : %s\n",buffer[i].seqno, buffer[i].payload);
                            if(buffer[i].isLast==1)
                                last=1;
                            shift++ ;
                        }
                        else break;
                    }
                    expt +=shift;
                    shiftbuff(buffer,shift);

                }
                else
                {
                    buffer[rcvpkt.seqno - expt].seqno = rcvpkt.seqno;
                    buffer[rcvpkt.seqno - expt].size = rcvpkt.size;
                    buffer[rcvpkt.seqno - expt].isLast = rcvpkt.isLast;
                    buffer[rcvpkt.seqno - expt].isData = rcvpkt.isData;
                    strcpy(buffer[rcvpkt.seqno - expt].payload,rcvpkt.payload);
                }
                // printf("data : %s\n", rcvpkt.payload);
                // else
                // {
                    sndpkt.seqno = rcvpkt.seqno;
                    sndpkt.isData = 0;
                    // sndpkt.
                    printf("SERVER  |  S  |  %s  |  ACK  |  %d  |  SERVER  |  Relay2\n", getTimeStamp(),rcvpkt.seqno);
                // printf("SENT ACK : %d\n", rcvpkt.seqno);
                    if(sendto(relayEven, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr1, slen1)==-1)
                    {
                      printf("sendto failed exiting...\n");
                      exit(0);
                    }
            }
        }

    }


}




