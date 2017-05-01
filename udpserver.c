/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* February 19, 2017 */

/* Updated by Alex Szostek */
/* May 3rd, 2017 */

/*
Steps of Execution
	1 Print out port number, ask user for config params
	2 $(Main Actions)
	3 Print to out.txt
*/

/*
Requirements
	> Out of sequence or duplicates are discarded
	> Output recieved data into file (out.txt)
	> Input files are 80 chars/line
	> Messages contain 1 line per packet
	> No checksums

Print Statements
	> Packet n recieved with c data bytes
	> Duplicate packet n recieved with c data bytes
	> Packet n lost
	> ACK n transmitted
	> ACK n lost
	> End of Transmission Packet with (sequence number n) recieved with c data bytes

Statistics
	> Number of data packets received successfully (without loss, without duplicates)
	> Total number of data bytes received which are delivered to user (this should be the sum of the count
	fields of all packets received successfully without loss without duplicates)
	> Total number of duplicate data packets received (without loss)
	> Number of data packets received but dropped due to loss
	> Total number of data packets received (including those that were successful, those lost, and duplicates)
	> Number of ACKs transmitted without loss
	> Number of ACKs generated but dropped due to loss
	> Total number of ACKs generated (with and without loss)
	> Total elapsed time from start to end in milliseconds as measured by calls to gettimeofday() (This
	 (time should be measured from the instant when the Receiver finishes opening the connection with
	 the Sender to the instant when the Receiver closes the output file.)

*/

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include "udpserver_utility.h" /* server utility structs/functions */

#define STRING_SIZE 1024
#define ACK_SIZE 2

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 65100

int main(void) 
{
   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
	float packet_loss_rate;
	float ack_loss_rate;

	//create stats
	stats s = create_stats();
	
	//get loss rates
	do
	{	
		printf("What should the packet loss rate be?\n");
		scanf("%f", &packet_loss_rate);
	} while(!(packet_loss_rate >= 0 && packet_loss_rate < 1));
	
	do
	{
		printf("What should the ACK loss rate be?\n");
		scanf("%f", &ack_loss_rate);
	} while(!(ack_loss_rate >= 0 && ack_loss_rate < 1)); 
   /* open a socket */
   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);

	/*
	Main Actions
		1 Wait for Packet
		2 When Packet arrives, check count = 0
			> Break if true
		3 Call SimulateLoss
			> Loop if 0 (loss)
		4 Process Packet
		5 Call SimulateACKLoss
			> If 1, generate ACK
		6 Loop
	*/

   for (;;) {

      bytes_recd = recvfrom(sock_server, &sentence, STRING_SIZE, 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);

		char count[] = {sentence[0], sentence[1], '\0'};
		char seq_number[] = {sentence[2], sentence[3], '\0'};
		
		printf("Packet %s recieved with %s data bytes\n", seq_number, count);
		s.total_recv_packets += 1;

		//break out of the loop if count = 0, EOT Packet
		if(count[1] == '0')
		{
			printf("End of Transmission Packet with Sequence Number %s recieved with %s data bytes\n", seq_number, count);
			s.successful_recv_packets += 1;
			break;
		}

		//if there was no loss
		if(simulate_loss(packet_loss_rate) == 1)
		{
			s.successful_recv_packets += 1;

			/* generate an ACK */
			msg_len = sizeof(char) * 2 + 1;
			char* ack = malloc(msg_len); 
			ack[0] = seq_number[0];
			ack[1] = seq_number[1];
			s.acks_generated += 1;
			printf("ACK %c generated\n", ack[0]);

			//if there was no ack loss
			if(simulate_loss(ack_loss_rate) == 1)	
			{
				/* send message */
				bytes_sent = sendto(sock_server, ack, msg_len, 0,
							(struct sockaddr*) &client_addr, client_addr_len);
				s.acks_without_loss += 1;
				printf("ACK %c transmitted\n", ack[0]);
			}
			//there was an ack loss
			else
			{
				printf("ACK %c lost\n", ack[0]);
				s.acks_lost += 1;
			}
		}
		//there was a packet loss
		else
		{
			printf("Packet %s lost\n", seq_number);
			s.lost_packets += 1;
		}
   }

	print_stats(s);
}
