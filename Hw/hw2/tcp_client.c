// Student Number : 2018110115 		Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 20

#define ERROR 0
#define SUCCESS 1

#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

typedef struct {
	int cmd;
	char addr[BUF_SIZE];
	struct in_addr iaddr;
	int result;
}PACKET;



void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
	PACKET send_packet;
	PACKET recv_packet;
	
	int rx_len=0;

	if (argc!=3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM,0);
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");

	char s[BUF_SIZE];

	while(1) {
		printf("Input dotted-decimal address: ");
		scanf("%s",s);
		
		if(strcmp(s,"quit")==0) { // client input quit
			printf("[Tx] cmd : 2(QUIT)\n");
			printf("Client socket close and exit\n");

			send_packet.cmd = QUIT;
			write(sock,(char*) &send_packet,sizeof(PACKET));
			break;
		}
		else { // client send dotted_deciaml address
			printf("[Tx] cmd: 0, addr: %s\n",s);

			strcpy(send_packet.addr,s);
			send_packet.cmd = REQUEST;
			write(sock,(char*) &send_packet,sizeof(PACKET));
		}
		
		// Now take the data from the server
		rx_len = read(sock, &recv_packet, sizeof(PACKET));
		if(rx_len == 0)
			break;

		if(recv_packet.cmd == RESPONSE) {
			if (recv_packet.result == SUCCESS) {
				printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n",recv_packet.cmd,recv_packet.iaddr.s_addr,recv_packet.result);

			}
			else if(recv_packet.result == ERROR) {
				printf("[Rx] cmd: %d, Address conversion: fail! (result: %d)\n",recv_packet.cmd,recv_packet.result);
			}
			else
			{
				printf("result Trash\n");
			}

		}
		else{
			printf("[Rx] invalid cmd : %d\n",recv_packet.cmd);	
		}
	
		printf("\n");
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

