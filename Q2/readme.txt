Name  :  Sargun
ID    :  2017A7PS0104P

- The window size has been defined as a macro WINDOW in packet.h
- The Timeout Duration(in secs) has been defined as a macro TIMEOUT in packet.h
- The packet size has been defined as a macro PACKET_SIZE in packet.h
- The Packet drop rate has been defined as a macro PDR in packet.h
- Server uses a buffer of size WINDOW.

- Client timer has been implemented for the whole window so if there is a timeout all unacked pkts in the window are resent.
- The files Q2_server.c Q2_relay1.c Q2_relay2.c and Q2_client.c need to be run on seperate terminals and they provide the log on the console
- Please run server and relay fns before clients.

** PORT MACRO IN SERVER SHOULD BE SAME AS PORT in RELAY1**
** PORT1 MACRO IN SERVER SHOULD BE SAME AS PORT in RELAY2**
** PORT1 IN CLIENT AND RELAY1 SHOULD BE SAME ** 
** PORT2 IN CLIENT AND PORT1 in RELAY2 SHOULD BE SAME ** 

- The relay files need to be force stopped as relay is implemented as being forever available.
- Client and server programs exit when the file transfer is complete

- client reads file named input.txt and server writes data received to a file named destination.txt.

COMPILE : 
gcc -o client client.c
gcc -o relay1 relay1.c
gcc -o relay2 relay2.c
gcc -o server server.c

RUN : 
./server
./relay1
./relay2
./client


