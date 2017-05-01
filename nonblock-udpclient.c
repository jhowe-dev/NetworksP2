/* nonblock-udp_client.c */ 
/* Programmed by Adarsh Sethi */
/* February 19, 2017 */

/* Updated by John Howe */
/* May 3rd, 2017 */

/* Version of udp_client.c that is nonblocking */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <fcntl.h>          /* for fcntl */
#include <sys/time.h>
#include <math.h>
#define STRING_SIZE 1024
int main(int argc, char* argv[]) {

   int sock_client;  /* Socket used by client */ 
   int timeout;
   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */

   int fcntl_flags; /* flags used by the fcntl function to set socket
                       for non-blocking operation */
	int packets_sent = 0;
	int total_bytes_sent = 0;
	int retransmitted_packets = 0;
	int total_packets_sent = 0; //including retrans
	int timeouts = 0;
	int acks_received = 0;
	double elapsed_time;
  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* make socket non-blocking */
   fcntl_flags = fcntl(sock_client, F_GETFL, 0);
   fcntl(sock_client, F_SETFL, fcntl_flags | O_NONBLOCK);

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);
  
   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /* user interface */
	
	char const* const fileName = argv[1]; /* should check that argc > 1 */
	int tout = atoi(argv[2]);
	double const timeout_value = pow(10.0, (double)tout);
	printf("%lf \n",timeout_value);
   
	FILE* file = fopen(fileName, "r"); /* should check the result */
   
	char line[80];
	char packet[84];

	char curSequence = '0';
	
	char currentAck;
	//TODO CLean this up.  Get those Doc points
	while (fgets(line, sizeof(line), file)) {
		//move the read line into the packet array
		memcpy(packet + 4, line, 80);
		
		//TODO add in correct size info..
		packet[0] = '0';
		packet[1] = '5';
		int str = (int) strlen(line);
		int firstDigit = 0;
		int lastDigit = 0;
		printf("%d \n", str);
		if(str > 10)
		{
			firstDigit = str / 10;
			lastDigit = str % 10;	
			printf("Tens: %d, ones:%d \n",firstDigit,lastDigit);
		}
		else
		{
			lastDigit = str % 10;
			printf("Ones: %d \n",lastDigit);
		}
		char first = '0' + firstDigit;
		char last = '0' + lastDigit;
		packet[0] = first;
		packet[1] = last;
		printf("Packet: %s\n",packet);
		//Sequence number
		packet[2] = curSequence;
		packet[3] = '0';

		msg_len = sizeof(packet) +1;
		//debug info, shows packet structure
		printf("%s \n", packet);

		/*Create the variables to store the times before starting time!*/
		//Stores the Current time during wait
		double cts;
		double ctm;
		double ct;
		double difference;
		
		//indicates a timeout
		int timeout_flag = 0;

		//structs for times
		struct timeval startTime;
		struct timeval currentTime;

		//initially send the packet
		bytes_sent = sendto(sock_client, packet, sizeof(packet) +1, 0,
				(struct sockaddr *) &server_addr, sizeof (server_addr));
		
		//get initial time
		gettimeofday(&startTime,NULL);
		
		//Used to store the times for calculating timeouts
		double sts = startTime.tv_sec * 1000000.0;
		double stm = startTime.tv_usec;
		double st = stm + sts;
						
		do {  /* loop required because socket is nonblocking */
			if(timeout_flag == 1)
			{
				bytes_sent = sendto(sock_client, packet, sizeof(packet) +1, 0,
				(struct sockaddr *) &server_addr, sizeof (server_addr));
	
				printf("Packet resent: %s \n",packet);	
				
				//re-get time
				gettimeofday(&startTime,NULL);				
				
				//recalculate start time
				sts = startTime.tv_sec * 1000000.0;//convert seconds to usec
				stm = startTime.tv_usec;
				st = stm + sts;
				timeout_flag = 0;
			}//retransmit packet on loss (of any kind)
			
			//non-blocking receive 
			bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
						 (struct sockaddr *) 0, (int *) 0);
			
			//get the current time for timeout calculation
			gettimeofday(&currentTime, NULL);

			//calculate current time (in usec)
			cts = currentTime.tv_sec * 1000000.0;
			ctm = currentTime.tv_usec;
			ct = cts + ctm;
			
			//elapsed time (in usec)
			difference = ct - st;
			
			//check for timeout			
			if(difference > timeout_value)
			{
				printf("Timeout Detected! \n");
				//debug info ---
				printf("Time Taken: %lf Threshold: %lf \n", difference, timeout_value);
				//---TODO remove
				
				printf("Retransmitting packet!...\n");
				
				bytes_recd = 0; // ensure that the packet gets resent;
				
				//set the resend flag
				timeout_flag = 1;

				//for the final statistics
				timeouts++;
				retransmitted_packets++;
			}
			else{
				//debug info
				printf("Current time: %lf ; Start Time: %lf \n", ct, st);
			}
			/* Note: you can do something else in this loop while
				waiting for a response from the server
			*/
		}
		while (bytes_recd <= 0);
		
		printf("Success on packet \n");	
		
		//flip sequence number
		if(curSequence == '0')
		{curSequence = '1';}
		else
		{curSequence = '0';}
		
		printf("\nThe server responded!\n");//TODO print ACK number or something
    }
	
	char goodbye[4];
	goodbye[0]='0';
	goodbye[1]='0';
	goodbye[2]=curSequence;
	goodbye[3]=0;
   fclose(file);
	bytes_sent = sendto(sock_client, goodbye, sizeof(goodbye)+1, 0,
				(struct sockaddr *) &server_addr, sizeof (server_addr));
  
   
   /* close the socket */

   close (sock_client);
}//main

