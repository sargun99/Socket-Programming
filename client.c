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




int main(void)
{
	int client0 = 0,client1 = 0; // dono sockets integer honge
	int activity;// select ke liye... 
    int bytesReceived = 0;
    int SEQ_NO = 0;
    struct timeval cl0,cl1;
    int send_new = 1;
    int nread0,nread1;
    // char recvBuff[256];
    // memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr,serv_addr2;// dono socket ke liye struct .
    int slen = sizeof(serv_addr);


    /*pehla banao*/
	if((client0 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /*doosra banao*/
    if((client1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    // serv_addr2.sin_family = AF_INET;
    // serv_addr2.sin_port = htons(PORT); // port
    // serv_addr2.sin_addr.s_addr = inet_addr("127.0.0.1");

    // printf("%d    :::::::    %d\n",client0,client1); 

    //CONNECTIONS//
    if(connect(client0, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    if(connect(client1, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    PKT spkt1,spkt0,rpkt1,rpkt0;

    // printf("BOTH CONNECTIONS DONE....\n");

    FILE *fp = fopen("input.txt","rb"); // rb nhi krna haina?? puch le.
    if(fp==NULL)
    {
        printf("File opern error");
        return 1;   
    }

    //char Buff0[PACKET_SIZE];
    if(send_new == 1)
    {
	    memset(spkt0.payload, '0', sizeof(spkt0.payload));
	    nread0 = fread(spkt0.payload,1,PACKET_SIZE,fp);
	    //make pkt + send
	    // printf("MAKING SEND PKT\n");
	    spkt0.size = nread0;
	    spkt0.seqno = SEQ_NO;
	    SEQ_NO += nread0;
	    spkt0.isLast = 0;
	    if(nread0 < PACKET_SIZE)
	    {
	    	if (feof(fp))
	        {
	        	// printf("End of file 1\n");
	        	spkt0.isLast = 1;
	        	send_new = 0;
	        	// exit(0);
	        }

	        if (ferror(fp))
	            printf("Error reading\n");
	        // exit(0);
	    }
	    spkt0.isData = 1;
	    spkt0.channel = 0;
	    
	    if(send(client0, &spkt0, sizeof(spkt0), 0)==-1)
		{
		  printf("send failed exiting...\n");
		  exit(0);
		}
		printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 0\n\n",spkt0.seqno,spkt0.size);
		// printf("PKT SENT\n");
		gettimeofday(&cl0, NULL);
		if(send_new == 0)
		{
			//rcv and exit!!
			// printf("PEHLA HI LAST HAI IMPLEMENT THIS...\n");

			fd_set readfds1;	
			struct timeval t1;
			t1.tv_usec=0;
			t1.tv_sec = TIMEOUT;
			while(1)
			{
				FD_ZERO(&readfds1);
				FD_SET(client0,&readfds1);
				
				if(select(client0+1,&readfds1,NULL,NULL,&t1))
				{
					if (recv(client0, &rpkt0, sizeof(rpkt0), 0) == -1)
	        		{
	            		  printf("Receive error!!! exit...\n");
	            		  exit(0);
	        		}
	        		printf("RCVD ACK : For PKT with Seq.No. : %d, from channel 0\n\n",rpkt0.seqno);
	        		exit(0);
				}


			}

		}

    }

    //char Buff1[PACKET_SIZE];
    if(send_new == 1)
    {
	    memset(spkt1.payload, '0', sizeof(spkt1.payload));
	    nread1 = fread(spkt1.payload,1,PACKET_SIZE,fp);
	    //send
		//make pkt + send
		// printf("MAKINGG pkt second conn\n");
	    spkt1.size = nread1;
	    spkt1.seqno = SEQ_NO;
	    spkt1.isLast = 0;
	    SEQ_NO += nread1;
	    if(nread1 < PACKET_SIZE)
	    {
	    	if (feof(fp))
	        {
	        	// printf("End of file 2\n");
	        	spkt1.isLast = 1;
	        	send_new = 0;
	        	// exit(0);
	        }

	        if (ferror(fp))
	            printf("Error reading\n");
	        // exit(0);
	    }
	    spkt1.isData = 1;
	    spkt1.channel = 1;
	    
	    if(send(client1, &spkt1, sizeof(spkt1), 0)==-1)
		{
		  printf("send failed exiting...\n");
		  exit(0);
		}
		printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 1\n\n",spkt1.seqno,spkt1.size);
		// printf("PKT SENT\n");
		gettimeofday(&cl1, NULL);
	}
    //Now 2 clients connected to server diff sockets and ek ek bhej chuke.
    //jiska bhi packet aayega uske liye aage bhejna hai....

    fd_set readfds;	
    // FD_SET(client0, &readfds);  
    // FD_SET(client1, &readfds); 

    //1ms timeout
    struct timeval t;
    t.tv_sec=0;
    t.tv_usec=100000;
    struct timeval tmp;
    // struct timeval cl0,cl1,tmp,timeoutDur;
    // timeoutDur.tv_sec = TIMEOUT;
    // timeoutDur.tv_usec = 0;
    int closed0 = 0;
    int closed1 = 0;

    // int mval=client0; // select ka 1st arg.
    // if(client1>mval)
    // 	mval=client1;
    int iter =0;
    int fl=0,fl1=0;
    while((closed1 == 0 || closed0 == 0))
    {
    	iter++;

    	FD_ZERO(&readfds);
    	FD_SET(client0, &readfds);
    	FD_SET(client1, &readfds); 
    	int mval = client0; // select ka 1st arg.
	    if(client1 > mval)
	    	mval = client1;

    	// printf("IN WHILE %d   %d    send_new : %d\n\n",closed0,closed1,send_new);
	    activity = select( mval+1 , &readfds , NULL , NULL , &t);
	    // printf("%d  : %d :  %d   mval: %d\n", activity,client0,client1,mval);
	    // if(FD_ISSET(client0, &readfds))
    	// {
    	// 	printf("zero set h\n");
    	// }
    	// if(FD_ISSET(client1, &readfds))
    	// {
    	// 	printf("one set h\n");
    	// }   
	    // printf("IN WHILE AFTER SELECT\n");
	    if ((activity < 0))   
	    {   
	        printf("select error\n");
	    }
	    else if(activity == 0)
	    {
	    	// timeout...dono phas gaye
	    	// printf("LLLLLLLLLLLL   dono phase h  LLLLLLLLLLLLL\n");
	    	fl=0;fl1=0;	    	

	    	gettimeofday(&tmp, NULL);	    	
	    	if(tmp.tv_sec - cl0.tv_sec > TIMEOUT)
	    		fl=1;
	    	else if(tmp.tv_sec - cl0.tv_sec == TIMEOUT && tmp.tv_usec >= cl0.tv_usec)
	    		fl=1;

	    	if(fl == 1 && closed0 == 0)
	    	{
	    		// printf("Both timeout resend channel 0\n");
	    		if(send(client0, &spkt0, sizeof(spkt0), 0)==-1)
				{
				  printf("send failed exiting...\n");
				  exit(0);
				}
				printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 0\n\n",spkt0.seqno,spkt0.size);
	    		gettimeofday(&cl0, NULL);

	    	}

	    	gettimeofday(&tmp, NULL);
	    	if(tmp.tv_sec - cl1.tv_sec > TIMEOUT)
	    		fl1=1;
	    	else if(tmp.tv_sec - cl1.tv_sec == TIMEOUT && tmp.tv_usec >= cl1.tv_usec)
	    		fl1=1;

	    	if(fl1 == 1&& closed1 == 0)
	    	{
	    		// printf("Both timeout resend channel 1\n");
	    		//resend
	    		if(send(client1, &spkt1, sizeof(spkt1), 0)==-1)
				{
				  printf("send failed exiting...\n");
				  exit(0);
				}
				printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 1\n\n",spkt1.seqno,spkt1.size);

	    		gettimeofday(&cl1, NULL);
	    	}

	    }
	    else
	    {
	    	int fl=0,fl1=0;
	    	// printf("inside else \n");
	    	

	    	if(FD_ISSET(client0, &readfds) && closed0 == 0)
	    	{
	    		//new
	    		// printf("waiting to rcv\n");
	    		// printf("got ack cl0\n");
	    		if (recv(client0, &rpkt0, sizeof(rpkt0), 0) == -1)
        		{
            		  printf("Receive error!!! exit...\n");
            		  exit(0);
        		}
        		printf("RCVD ACK : For PKT with Seq.No. : %d, from channel 0\n\n",rpkt0.seqno);
        		// printf("done rcv\n");
	    		if(send_new == 1)
	    		{
		    		memset(spkt0.payload, '0', sizeof(spkt0.payload));
				    nread0 = fread(spkt0.payload,1,PACKET_SIZE,fp);
				    //make pkt + send
				    spkt0.size = nread0;
				    spkt0.seqno = SEQ_NO;
				    spkt0.isLast = 0;
				    SEQ_NO += nread0;
				    if(nread0 < PACKET_SIZE)
				    {
				    	if (feof(fp))
				        {
				        	// printf("End of file 3\n");
				        	spkt0.isLast = 1;
				        	send_new = 0;
				        	// exit(0);
				        }

				        if (ferror(fp))
				            printf("Error reading\n");
				        // exit(0);
				    }
				    spkt0.isData = 1;
				    spkt0.channel = 0;
				    // printf("Sending packet!!\n");
				    if(send(client0, &spkt0, sizeof(spkt0), 0)==-1)
					{
					  printf("send failed exiting...\n");
					  exit(0);
					}
					printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 0\n\n",spkt0.seqno,spkt0.size);
					gettimeofday(&cl0, NULL);
					
				}
				else
				{
					
	        		// printf("CONN 0 KA HOGAYA\n");
	        		closed0 = 1;
	        		// close(client0);

				}

	    	}
	    	else
	    	{
	    		gettimeofday(&tmp, NULL);    	
	    		fl = 0;
		    	if(tmp.tv_sec - cl0.tv_sec > TIMEOUT)
		    		fl=1;
		    	else if(tmp.tv_sec - cl0.tv_sec == TIMEOUT && tmp.tv_usec >= cl0.tv_usec)
		    		fl=1;

		    	if(fl==1 && closed0 == 0)
		    	{
		    		// resend
		    		// printf("resend channel 0\n");
		    		if(send(client0, &spkt0, sizeof(spkt0), 0 )==-1)
					{
					  printf("send failed exiting...\n");
					  exit(0);
					}
					printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 0\n\n",spkt0.seqno,spkt0.size);
		    		gettimeofday(&cl0, NULL);

		    	}
	    	}
	    	

	    	if(FD_ISSET(client1, &readfds) && closed1 == 0)
	    	{
	    		//new
	    		// printf("got ack cl1 \n");
	    		if (recv(client1, &rpkt1, sizeof(rpkt1), 0) == -1)
        		{
            		  printf("Receive error!!! exit...\n");
            		  exit(0);
        		}
        		printf("RCVD ACK : For PKT with Seq.No. : %d, from channel 1\n\n",rpkt1.seqno);

	    		if(send_new == 1)
	    		{
		    		memset(spkt1.payload, '0', sizeof(spkt1.payload));
	    			nread1 = fread(spkt1.payload,1,PACKET_SIZE,fp);

		    		spkt1.size = nread1;
				    spkt1.seqno = SEQ_NO;
				    spkt1.isLast = 0;
				    SEQ_NO += nread1;
				    if(nread1 < PACKET_SIZE)
				    {
				    	if (feof(fp))
				        {
				        	// printf("End of file 4\n");
				        	spkt1.isLast = 1;
				        	send_new = 0;
				        	// exit(0);
				        }

				        if (ferror(fp))
				            printf("Error reading\n");
				        // exit(0);
				    }
				    spkt1.isData = 1;
				    spkt1.channel = 1;
				    // printf("sending packet channel 1\n");
				    if(send(client1, &spkt1, sizeof(spkt1), 0)==-1)
					{
					  printf("send failed exiting...\n");
					  exit(0);
					}
					gettimeofday(&cl1, NULL);
					printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 1\n\n",spkt1.seqno,spkt1.size);
				}
				else
				{
					
	        		// printf("CONN 1 KA HOGAYA\n");
	        		closed1 = 1;
	        		// close(client1);
				}
	    	}
	    	else
	    	{
	    		fl1 = 0;
	    		gettimeofday(&tmp, NULL);
		    	if(tmp.tv_sec - cl1.tv_sec>TIMEOUT)
		    		fl1=1;
		    	else if(tmp.tv_sec - cl1.tv_sec == TIMEOUT && tmp.tv_usec >= cl1.tv_usec)
		    		fl1=1;

		    	if(fl1==1 && closed1 == 0)
		    	{
		    		//resend
		    		printf("resend channel 1\n");
		    		if(send(client1, &spkt1, sizeof(spkt1), 0)==-1)
					{
					  printf("send failed exiting...\n");
					  exit(0);
					}
					printf("SENT PKT : Seq.No. : %d, of size : %d, from channel 1\n\n",spkt1.seqno,spkt1.size);
		    		gettimeofday(&cl1, NULL);
		    	}
	    	}
	    	
	    }
	    // if(closed1==1 && closed0==1)
	    // {
	    // 	printf("DONE\n");
	    // 	exit(0);
	    // }

    }
}































