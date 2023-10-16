// Student Number : 2018110115 		Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 20

// result filed value
#define ERROR 0
#define SUCCESS 1

// cmd filed value
#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

typedef struct {
	int cmd;						// 0: request, 1: response , 2 : quit
	char addr[BUF_SIZE];			// dotted-decimal address(20 bytes)
	struct in_addr iaddr;			// inet_aton()  changed adddress stored
	int result;						// 0: Error, 1: Success
}PACKET;


void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
	PACKET send_packet;
	PACKET recv_packet;
	int rx_len = 0;
	
	if(argc!=2){
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}
	serv_sock=socket(PF_INET, SOCK_STREAM,0);
	if(serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock,5) == -1)
		error_handling("listen() error");

	clnt_addr_size=sizeof(clnt_addr);
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);

	if(clnt_sock==-1)
		error_handling("accept() error");

	printf("--------------------------\n");
	printf("Address Conversion Server\n");
	printf("--------------------------\n");

	while(1) {
		rx_len = read(clnt_sock, &recv_packet, sizeof(PACKET));
		if (rx_len == 0)
			break;
		
		if(recv_packet.cmd == REQUEST) {
			printf("[RX] Received Dotted_Decimal Address: %s\n", recv_packet.addr);

			if(inet_aton(recv_packet.addr, &send_packet.iaddr)==0) {
				// address transform fail
				printf("[Tx] Address conversion fail:(%s)\n", recv_packet.addr);
			
				send_packet.result = ERROR;
				send_packet.cmd = RESPONSE;
				write(clnt_sock,(char*) &send_packet, sizeof(PACKET));

			}
			else {
				// address transform sucess
				printf("inet_ation(%s) -> %#x \n", recv_packet.addr, send_packet.iaddr.s_addr);
				send_packet.cmd = RESPONSE; send_packet.result = SUCCESS;
				printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n",send_packet.cmd,send_packet.iaddr.s_addr,send_packet.result);
				write(clnt_sock, (char*) &send_packet, sizeof(PACKET));
			}

		}
		else if(recv_packet.cmd == QUIT) {
				
			printf("QUIT message received\n");
			printf("Server socket close and exit.\n");

			break;
		}
		else {
			printf("[Rx] Invalid command: %d\n", recv_packet.cmd);
		}
		printf("\n");

	}

	close(clnt_sock);
	close(serv_sock);


	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

