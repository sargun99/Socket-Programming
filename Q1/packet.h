#include<stdio.h>
#include<stdlib.h>

#define PACKET_SIZE 100
#define PORT 8888 
#define PDR 10
#define BUFF_SIZE 10
#define TIMEOUT 2

typedef struct packet1{
    int size;//of payload;
    int seqno;//offset from byte 1
    int isLast;
    int isData;
    int channel;
    char payload[PACKET_SIZE];
}PKT;

