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

#define PORT1 8884
#define PORT2 8885



void shiftack(int ack[],int shift)
{
	int i;
	for(i=0;i + shift < WINDOW;i++)
	{
		ack[i] = ack[i+shift];
	}
	for(;i<WINDOW;i++)
	{
		ack[i]=0;
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



void shiftcopy(PKT copy[],int shift)
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

int main(void)
{
	int Odd = 0;
	int Even = 0 ;
	struct sockaddr_in addr1,addr2;

	if((Odd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT1); // port
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((Even = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(PORT2); // port
    addr2.sin_addr.s_addr = inet_addr("127.0.0.1");
    int slen1=sizeof(addr1),slen2=sizeof(addr2);

    PKT sndpkt,rcvpkt;
    // printf("GOIN IN\n");
    int i = 0;


    FILE *fp = fopen("input.txt","rb"); // rb nhi krna haina?? puch le.
    if(fp == NULL)
    {
        printf("File open error");
        return 1;
    }

    int curr = 0,seq = 0;
    int winSt = 0,winEnd = WINDOW;
    int nread,more =1;
    PKT copy[WINDOW];

    char pay[ PACKET_SIZE ];
    struct timeval sel,timer,now;
    while(curr < winEnd && more == 1)
    {
    	sndpkt.seqno = curr;
    	sndpkt.isLast = 0;
    	memset(sndpkt.payload, '\0', sizeof(sndpkt.payload));
	    nread = fread(sndpkt.payload,1,PACKET_SIZE ,fp);
	    // strcpy(sndpkt.payload,pay);
	    // printf("NREAD :: %d\n", nread);
	    sndpkt.isData = 1;
	    sndpkt.size = nread;
	    // printf("%s , %ld\n", sndpkt.payload , sizeof(sndpkt.payload));
	    // printf("\n\n%s\n\n", pay);
	    if(nread < PACKET_SIZE)
	    {
	    	if(feof(fp))
	    	{
	    		sndpkt.isLast = 1;
	    		more = 0;
	    	}

    		if (ferror(fp))
            printf("Error reading\n");
	    }

	    if(curr % 2 == 1)
	    {
	    	printf("CLIENT  |  S  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay1\n", getTimeStamp(),sndpkt.seqno);
	    	if(sendto(Odd, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr1, slen1)==-1)
	        {
	          printf("sendto failed exiting...\n");
	          exit(0);
	        }
	    }
	    else
	    {
	    	printf("CLIENT  |  S  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay2\n", getTimeStamp(),sndpkt.seqno);
	    	if(sendto(Even, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr2, slen2)==-1)
	        {
	          printf("sendto failed exiting...\n");
	          exit(0);
	        }
	    }
	    copy[curr-winSt].seqno = sndpkt.seqno;
	    copy[curr-winSt].isLast = sndpkt.isLast;
	    copy[curr-winSt].isData = sndpkt.isData;
	    copy[curr-winSt].size = sndpkt.size;
	    strcpy(copy[curr-winSt].payload , sndpkt.payload);
	    curr++;
	    gettimeofday(&timer, NULL);	

    }
    fd_set readfds; 
    int max = Odd;
    if(Even > max)
        max = Even;

    int ack[WINDOW];
    memset(ack, 0, WINDOW *sizeof(ack[0]));

    

    sel.tv_sec = 0;
    sel.tv_usec = 100; 

    while(1)
    {
    	// printf("WHILE  curr : %d  %d  %d\n", curr , winSt , more);
    	// i++;

    	FD_ZERO(&readfds);
 
        FD_SET( Even , &readfds);

        FD_SET( Odd , &readfds);
        int activity = select( max + 1 , &readfds , NULL , NULL , &sel); 
        // if(activity>0)  
        // printf("%d\n", activity);
        if ((activity < 0) ) //&& (errno!=EINTR))   
        {   
            printf("select error");   
        }
        /* JO ACK MILE LELO */
        if(FD_ISSET( Odd , &readfds))
        {
        	
        	if(recvfrom(Odd, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr1, &slen1) == -1)
	        {
	            printf("recvfrom error..exit\n");
	            exit(0);
	        }
	        printf("CLIENT  |  R  |  %s  |  ACK  |  %d  |  Relay1  |  CLIENT\n", getTimeStamp(),rcvpkt.seqno);
	        ack[rcvpkt.seqno-winSt] = 1;

        }

        if(FD_ISSET(Even , &readfds))
        {
        	if(recvfrom(Even, &rcvpkt, sizeof(rcvpkt), 0,(struct sockaddr *) &addr2, &slen2) == -1)
	        {
	            printf("recvfrom error..exit\n");
	            exit(0);
	        }
	        printf("CLIENT  |  R  |  %s  |  ACK  |  %d  |  Relay2  |  CLIENT\n", getTimeStamp(),rcvpkt.seqno);
	        ack[rcvpkt.seqno-winSt] = 1;
        }
        /* JO ACK MILE LELO */

        /*SHIFTING */
        int shift = 0;
        for(i=0;i<WINDOW;i++)
        {
        	if(ack[i]==1)
        		shift++;
        	else break;
        }
        // if(shift>0)
        // {
        // 	printf("%d\n", shift);
        // 	for(i=0;i<WINDOW;i++)
        // 		printf("%d ", ack[i]);
        // 	printf("\n");
        	
        // 	for(i=0;i<WINDOW;i++)
        // 		printf("%d ", ack[i]);
        // 	printf("\n");
        // }

        shiftack(ack,shift);
        // shift copy tooo....aage krte hai isse bhi
        shiftcopy(copy,shift);

        // printf("\n COPY ELEMENTS\n");
        // for(i=0;i<WINDOW;i++)
        // {
        // 	printf("%d ", copy[i].seqno);
        // }
        // printf("\n");

        winEnd += shift;
        winSt += shift;
        /*SHIFTING */

        /* Ab jitni window me jagah hai utne pkts bhej de */
        while(curr < winEnd && more == 1)
	    {
	    	sndpkt.seqno = curr;
	    	sndpkt.isLast = 0;
	    	memset(sndpkt.payload, '\0', sizeof(sndpkt.payload));
	    	nread = fread(sndpkt.payload,1,PACKET_SIZE ,fp);
		    // strcpy(sndpkt.payload,pay);
		    sndpkt.isData = 1;
		    sndpkt.size = nread;
		    // printf("NREAD :: %d\n", nread);
		    // printf("%s , %ld\n", sndpkt.payload , sizeof(sndpkt.payload));
		    if(nread < PACKET_SIZE)
		    {
		    	if(feof(fp))
		    	{
		    		sndpkt.isLast = 1;
		    		more = 0;
		    	}

		    	if (ferror(fp))
	            printf("Error reading\n");
		    }
		    // printf("%s\n", sndpkt.payload);
		    if(curr % 2 == 1)
		    {
		    	printf("CLIENT  |  S  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay1\n", getTimeStamp(),sndpkt.seqno);
		    	if(sendto(Odd, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr1, slen1)==-1)
		        {
		          printf("sendto failed exiting...\n");
		          exit(0);
		        }
		    }
		    else
		    {
		    	printf("CLIENT  |  S  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay1\n", getTimeStamp(),sndpkt.seqno);
		    	if(sendto(Even, &sndpkt, sizeof(sndpkt), 0,(struct sockaddr*) &addr2, slen2)==-1)
		        {
		          printf("sendto failed exiting...\n");
		          exit(0);
		        }
		    }
		    copy[curr-winSt].seqno = sndpkt.seqno;
		    copy[curr-winSt].isLast = sndpkt.isLast;
		    copy[curr-winSt].isData = sndpkt.isData;
		    copy[curr-winSt].size = sndpkt.size;
		    strcpy(copy[curr-winSt].payload , sndpkt.payload);
		    curr++;

		    gettimeofday(&timer, NULL);	    	

	    }
	    /* Ab jitni window me jagah hai utne pkts bhej de */

	    // sab acks aane pe curr = winSt aur file khatam pe more = 0 => EXIT!!! 
	    if(curr == winSt && more == 0)
	    	break;



	    /*RESEND KE LIYE */

	    gettimeofday(&now, NULL);
	    int fl =0;

	    if(now.tv_sec - timer.tv_sec > TIMEOUT)
    		fl=1;
    	else if(now.tv_sec - timer.tv_sec == TIMEOUT && now.tv_usec >= timer.tv_usec)
    		fl=1;

    	if(fl==1)
    	{
    		//resend all unacked ones...


    		for(i=0;i<WINDOW;i++)
    		{
    			if(copy[i].seqno >= 0 && ack[i] == 0)
    			{
    				if(copy[i].seqno %2 == 1)
    				{
    					printf("CLIENT  |  TO  |  %s  |  ACK  |  %d  |  Relay1  |  CLIENT\n", getTimeStamp(),copy[i].seqno);
    					printf("CLIENT  |  RE  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay1\n", getTimeStamp(),copy[i].seqno);
    					if(sendto(Odd, &copy[i], sizeof(copy[i]), 0,(struct sockaddr*) &addr1, slen1)==-1)
				        {
				          printf("sendto failed exiting...\n");
				          exit(0);
				        }
    				}
    				else
    				{
    					printf("CLIENT  |  TO  |  %s  |  ACK  |  %d  |  Relay2  |  CLIENT\n", getTimeStamp(),copy[i].seqno);
    					printf("CLIENT  |  RE  |  %s  |  DATA  |  %d  |  CLIENT  |  Relay2\n", getTimeStamp(),copy[i].seqno);
    					if(sendto(Even, &copy[i], sizeof(copy[i]), 0,(struct sockaddr*) &addr2, slen2)==-1)
				        {
				          printf("sendto failed exiting...\n");
				          exit(0);
				        }
    				}
    				gettimeofday(&timer, NULL);	
    			}
    		}
    	}



    	
    }

}

