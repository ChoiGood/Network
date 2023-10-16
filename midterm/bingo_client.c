// Student Number : 2018110115 				Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define ROW 4
#define COL 4

#define RAND_START 1 // Start random value
#define RAND_END 30  // END random value

// result field value
#define FAIL 0			// number Not 
#define SUCCESS 1		// number Ok
#define CHECKED 2		// number ok  but,,, exited 

// cmd field value
#define BINGO_REQ 0
#define BINGO_RES 1
#define BINGO_END 2

void error_handling(char *message);

// Client -> Server 
typedef struct {
	int cmd;			// BINGO_REQ
	int number;			// RANDOM CREATED NUMBER (1 ~ 30)
}REQ_PACKET;

// Server -> Client
typedef struct {
	int cmd;					// BINGO_RES, BINGO_END
	int number;					// Number chosed by User
	int board[ROW][COL];		// binggo correct board (0 or bingo number)
	int result;					// result from the server (FAIL, SUCCESS, CHECKED)
}RES_PACKET;

void PrintBingo(int a[][COL])
{
	printf("+-------------------+\n");
	for(int i=0;i<ROW;i++) {
		for(int j=0; j<COL; j++) {
			if(a[i][j] != 0) {
				printf("|%3d ",a[i][j]);
			} else {
				printf("|    ");
			}
		}
		printf("|\n");
		printf("+-------------------+\n");
	}
}

int main(int argc, char* argv[])
{
	int sock;
	REQ_PACKET send_packet;
	RES_PACKET recv_packet;
	memset(&send_packet,0,sizeof(send_packet));
	memset(&recv_packet,0,sizeof(recv_packet));
	//.. int pac_len
	socklen_t adr_sz;
	struct sockaddr_in serv_adr, from_adr;
	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	srand(time(NULL));
	int rand_num;
	adr_sz = sizeof(from_adr);

	while(1)
	{
		rand_num = (rand()% RAND_END) + RAND_START;   // Random Number Creat!!
		
		send_packet.cmd = BINGO_REQ;
		send_packet.number = rand_num;

		printf("Random number(1~30): %d\n",rand_num);
		printf("[Tx] BINGO_REQ(number: %d\n\n", send_packet.number);
		// Send Packet to Server
		sendto(sock, &send_packet, sizeof(send_packet),0, (struct sockaddr*)&serv_adr , sizeof(serv_adr));
		memset(&send_packet,0, sizeof(send_packet));
		
		sleep(1);
		
		// Receive Packet from Server
		recvfrom(sock,  &recv_packet, sizeof(recv_packet),0, (struct sockaddr*) &from_adr, &adr_sz);
		
		if(recv_packet.cmd == BINGO_RES)
		{
			printf("[Rx] BINGO_RES(number: %d, result: %d)\n",recv_packet.number, recv_packet.result);
			PrintBingo(recv_packet.board);
			memset(&recv_packet,0,sizeof(recv_packet));
		}
		else if(recv_packet.cmd == BINGO_END)
		{
			printf("[Rx] BINGO_END\n");
			PrintBingo(recv_packet.board);
			printf("Exit Client\n");
			memset(&recv_packet,0,sizeof(recv_packet));
			break;
		}
		else {
			printf("Packet has Wrong Cmd.. from server\n");
		}


	}

	close(sock);

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
