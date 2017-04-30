/* Created by Alex Szostek */
/* May 3rd, 2017 */
#include <stdlib.h> //rand, RAND_MAX

/*
 * Enumeration: bool
 * Values: false, true
 * Use: Represents boolean values
 */
typedef enum {false, true} bool;

/*
 * Simulate (Packet/ACK) Loss
 * Returns 0 if there was loss, 1 otherwise
 */
bool simulate_loss(float loss_rate)
{
	double result = (double)rand() / (double)RAND_MAX;
	return (result > loss_rate);
}

typedef struct 
{
	// Number of data packets received successfully (without loss, without duplicates)
	int successful_recv_packets;

	//Total number of data bytes received which are delivered to user (this should be the sum of the count
	//fields of all packets received successfully without loss without duplicates)
	int data_bytes_sent;

	//Total number of duplicate data packets received (without loss)
	int duplicate_packets;

	//Number of data packets received but dropped due to loss
	int lost_packets;

	//Total number of data packets received (including those that were successful, those lost, and duplicates)
	int total_recv_packets;

	//Total number of ACKs generated (with and without loss)
	int acks_generated;

	//Number of ACKs transmitted without loss
	int acks_without_loss;

	//Number of ACKs generated but dropped due to loss
	int acks_lost;

	/*Total elapsed time from start to end in milliseconds as measured by calls to gettimeofday() (This
	 (time should be measured from the instant when the Receiver finishes opening the connection with
	 the Sender to the instant when the Receiver closes the output file.)*/
	int total_time;
} 
stats;

stats create_stats()
{
	stats s;
	s.successful_recv_packets = 0;
	s.data_bytes_sent = 0;
	s.duplicate_packets = 0;
	s.lost_packets = 0;
	s.total_recv_packets = 0;
	s.acks_generated = 0;
	s.acks_without_loss = 0;
	s.acks_lost = 0;
	s.total_time = 0;
	return s;
}

void print_separator()
{
	printf("---------------------------------------------------\n");
}

void print_stats(stats s)
{
	print_separator();
	printf("Total Recieved Packets : %d\n", s.total_recv_packets);
	printf("Successful Recieved Packets : %d\n", s.successful_recv_packets);
	printf("Duplicate Packets : %d\n", s.duplicate_packets);
	printf("Data Bytes Sent : %d\n", s.data_bytes_sent);
	printf("Lost Packets : %d\n", s.lost_packets);
	printf("ACKS Generated : %d\n", s.acks_generated);
	printf("ACKS Lost : %d\n", s.acks_lost);
	printf("ACKS Generated Without Loss : %d\n", s.acks_without_loss);
	printf("Total Time : %d\n", s.total_time);
	print_separator();
}


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
*/
