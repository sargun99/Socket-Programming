Name  :  Sargun
ID    :  2017A7PS0104P

- The PORT has been defined as a macro PORT in packet.h
- The Timeout Duration(in secs) has been defined as a macro TIMEOUT in packet.h
- The packet size has been defined as a macro PACKET_SIZE in packet.h
- The Packet drop rate has been defined as a macro PDR in packet.h
- Server uses a buffer of size BUFF_SIZE (macro in packet.h). If buffer is full packet is dropped.

- client sends input.txt to the server who in turn sends the data to a file named destination.txt


compile : 
gcc -o client client.c
gcc -o server server.c

run : 
./server
./client

