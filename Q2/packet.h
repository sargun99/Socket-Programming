#include <stdio.h>
#include <stdlib.h>

#define PACKET_SIZE 100
#define PDR 10
#define WINDOW 7
#define TIMEOUT 2

typedef struct packet1{
	int size;
	int isLast;
	int seqno;
	int isData;
	char payload[PACKET_SIZE + 1];
}PKT;




