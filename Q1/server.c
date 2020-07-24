#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <time.h>
#include "packet.h"
      
 



typedef struct buffer1{
	int seqno;
	int size;
	char payload[PACKET_SIZE];
}BUFFER;
     
int main(void)   
{   
    int opt = 1;   
    int master , addrlen , new_socket , client[2] ,clients = 2 , activity, i, sd;   
    int max;
    struct sockaddr_in addr,incoming; 
    int slen = sizeof(incoming);   
    int master1;
    
    fd_set readfds;    
     srand(time(0));
    //initialise all client_socket[] to 0 so not checked  
    client[0] = 0;
    client[1] = 0;
         
    //create a master socket  
    if( (master = socket(PF_INET , SOCK_STREAM , IPPROTO_TCP)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }

    // printf("master : %d  -------  	\n",master);
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    addr.sin_family = AF_INET;   
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   
    addr.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888
    if (bind(master, (struct sockaddr *)&addr, sizeof(addr))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    
         
    //try to specify maximum of 3(now 2) pending connections for the master socket  
    if (listen(master, 10) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(addr);   
    // puts("Waiting for connections ...");   
    int fl = 1;
    int isPending =1;


    PKT ack,rcvpkt,tmp;
    tmp.size = 0;
    tmp.seqno = -1;
    tmp.isLast = 0;
    tmp.isData = 0;


    
    if ((new_socket = accept(master,(struct sockaddr *)&addr, (socklen_t*)&addrlen))<0)   
    {   
        perror("accept");
        exit(EXIT_FAILURE);   
    }   
    // printf("NEW connection\n");
    client[0] = new_socket;

    
    if ((new_socket = accept(master,(struct sockaddr *)&addr, (socklen_t*)&addrlen))<0)   
    {   
        perror("accept");
        exit(EXIT_FAILURE);   
    }
    // printf("NEW connection\n");
    client[1] = new_socket;

    BUFFER buff[BUFF_SIZE];
    int buffIndex = 0;

    FILE *fp = fopen("destination.txt","wb"); // wb nhi krna haina?? puch le.
    if(fp==NULL)
    {
        printf("File open error");
        return 1;   
    }
    int expt = 0;

    while(fl)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        // FD_SET(master, &readfds);   
        // max = master;   
             max = 0;
        //add child sockets to set         
            
        // if(client[0] > 0)   
            FD_SET( client[0] , &readfds);                 
        
        if(client[0] > max)   
            max = client[0];        
        
        // if(client[1] > 0)   
            FD_SET( client[1] , &readfds);                 
        
        if(client[1] > max)   
            max = client[1];
        

     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max + 1 , &readfds , NULL , NULL , NULL);   
       	// printf("act : %d\n",activity);
        if ((activity < 0) ) //&& (errno!=EINTR))   
        {   
            printf("select error");   
        }              
        //If something happened on the master socket ,  
        //then its an incoming connection  
        // if (FD_ISSET(master, &readfds))   
        // {   
        //     printf("NEW connection\n");
        //     if ((new_socket = accept(master,(struct sockaddr *)&addr, (socklen_t*)&addrlen))<0)   
        //     {   
        //         perror("accept");
        //         exit(EXIT_FAILURE);   
        //     }   
            
        //     for (i = 0; i < clients; i++)   
        //     {   
        //         //if position is empty  
        //         if( client[i] == 0 )   
        //         {   
        //             client[i] = new_socket;
        //             break;   
        //         }   
        //     }   
        // }   
             
        //else its some IO operation on some other socket 
        for (i = 0; i < clients; i++)
        {   
            sd = client[i];
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                                
            	rcvpkt = tmp;
                //receive...
                if(recv(sd, &rcvpkt, sizeof(rcvpkt), 0) == -1)
	    		{
	        	    printf("recvfrom error..exit\n");
	        	    exit(0);
	    		}

	    		if(rcvpkt.seqno == -1)
	    		{fl=0;break;}


	    		// printf("NEW PKT RCVD %d  SIZE : %d\n", i,rcvpkt.size);
	    		// printf("%s\n", rcvpkt.payload);
	    		// printf("%d\n", rcvpkt.seqno);
                ack.size = 0;
                ack.seqno = rcvpkt.seqno;
                ack.isLast = 0;//dont matter...
                ack.isData = 0;
                ack.channel = i;
                
                

                int a = rand();
                // printf("gevd    ::   %d\n", a);
                int dontSend = 0 ;
                a%=100;
                a+=1;
                if(a <= PDR)
                	dontSend = 1;

                if(dontSend == 0)
                {
                	if(rcvpkt.seqno == expt)
					{
						//write to file and write whole buffer.
						fwrite(rcvpkt.payload, 1 , rcvpkt.size , fp);
						expt += rcvpkt.size;
						for( int p = 0 ; p < buffIndex; p++)
						{
							fwrite(buff[p].payload, 1 , buff[p].size , fp);							
							expt += buff[p].size;
						}
						buffIndex = 0;
					}
					else
					{
						// write to buffer;
						if(buffIndex == BUFF_SIZE)
						{
							dontSend = 1;
							// printf("!!!!!!!!!!!!!!!!!!!!!!!BUFFER FULL CASE !!!!!!!!!!\n");
						}
						else
						{
							buff[buffIndex].seqno = rcvpkt.seqno;
							buff[buffIndex].size = rcvpkt.size;
							strcpy(buff[buffIndex].payload,rcvpkt.payload);
							buffIndex++;
						}

					}
                }

                if(dontSend == 0)
                {
	                // printf("sending ack %d   :   %d \n\n", i,sd);
	                if(send(sd, &ack, sizeof(ack), 0)==-1)
					{
					  printf("sendto failed exiting...\n");
					  exit(0);
					}
					printf("RCVD PKT : Seq.No. %d of size %d Bytes from channel %d\n\n",rcvpkt.seqno ,rcvpkt.size ,i);
					printf("SENT ACK : for PKT with Seq.No. %d from channel %d\n\n", ack.seqno,i);
					
				}
				else
				{
					//packet lost
                    ;
                    // printf("PKT LOSS %d   :   %d \n\n", rcvpkt.seqno,i);
				}

				// if(rcvpkt.isLast == 1 && isPending == 0)
				// 	fl=0;
				//buffer vagera kro...s
                   
            }   
        }   
    }   
         
    return 0;   
}   