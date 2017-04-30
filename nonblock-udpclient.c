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
	int packets_sent;
	int total_bytes_sent;
	int retrans;
	int total_packets_sent; //including retrans
	int timeouts;
	int acks_received;
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


	struct timeval startTime;
	struct timeval currentTime;
   while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		memcpy(packet + 4, line, 80);
		
		packet[0] = '0';
		packet[1] = '0';
		packet[2] = curSequence;
		packet[3] = '0';

		msg_len = sizeof(packet) +1;
		printf("%s \n", packet);
		bytes_sent = sendto(sock_client, packet, msg_len, 0,
			(struct sockaddr *) &server_addr, sizeof (server_addr));
		if(curSequence == '0')
		{
			curSequence = '1';
		}
		else
		{
			curSequence = '0';
		}

		printf("Waiting for response from server...\n");
		gettimeofday(&startTime,NULL);
		int sts = startTime.tv_sec;
		int stm = startTime.tv_usec;
		printf("Seconds: %d \n",sts);
		printf("MSeconds: %d \n", stm);
		do {  /* loop required because socket is nonblocking */
			bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
						 (struct sockaddr *) 0, (int *) 0);

			/* Note: you can do something else in this loop while
				waiting for a response from the server
			*/
		}
		while (bytes_recd <= 0);

		printf("\nThe response from server is:\n");
		printf("%s\n\n", modifiedSentence);

    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);
	/* get response from server */
  
   
   /* close the socket */

   close (sock_client);
}
