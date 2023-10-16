// Student Number : 2018110115 		Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define SEQ_START 1000

#define FILEREQUEST 50
#define FILENOT 1
#define FILENAMELENMAX 100
typedef struct {
	int seq;					// SEQ number
	int ack;					// ACK number
	int buf_len;				// File read/write bytes
	char buf[BUF_SIZE];			// file name or file content
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

	printf("---------------------------------------\n");
	printf("   File Transmission Server\n");
	printf("---------------------------------------\n");

	int fd;
	int f_len;
	int total_len=0;
	char filename[FILENAMELENMAX];
	while(1) {
		rx_len = read(clnt_sock, &recv_packet, sizeof(PACKET));

		if(recv_packet.seq==FILEREQUEST) {
			fd=open(recv_packet.buf,O_RDONLY);
			if(fd==-1) {  // file does not exist
				strcpy(send_packet.buf,"File Not Found");
				printf("%s %s\n",recv_packet.buf, send_packet.buf);
				send_packet.seq = FILENOT;
				write(clnt_sock,(PACKET*) &send_packet, sizeof(PACKET));

				close(fd); close(clnt_sock); close(serv_sock);
				exit(1);
			}
			else {  // file exist!!
				strcpy(filename,recv_packet.buf);
				printf("[Server] sending %s\n\n",filename);
				send_packet.seq = SEQ_START;
				if((f_len=read(fd,send_packet.buf,BUF_SIZE))==-1)  // file read
					error_handling("file read() error!");
				total_len += f_len;

				printf("[Server] Tx: SEQ: %d, %d byte data\n", SEQ_START, f_len);

				write(clnt_sock,(PACKET*) &send_packet, sizeof(PACKET));	// sned PACKET to client

				if(f_len < 100) {    // if file < 100 byte, then we send and quit
					printf("%s sent (%d Bytes)\n",filename,total_len);
					
					close(fd);
					close(clnt_sock); close(serv_sock);
					exit(1);
				}
				else {
					break;
				}

				
			}
		}
	}

	while(1) { // this loof recv ACK ,, SEND SEQ and byte data

		rx_len = read(clnt_sock, &recv_packet, sizeof(PACKET));  // recv the ack from client
		if (rx_len == 0)
			break;
		printf("[Server] Rx ACK: %d\n\n",recv_packet.ack);

		if(recv_packet.ack == (send_packet.seq + BUF_SIZE)) {
			send_packet.seq = recv_packet.ack;              
			if ((f_len=read(fd,send_packet.buf,BUF_SIZE))==-1) // file read
				error_handling("file read() error");
			total_len += f_len;

			printf("[Server] Tx: SEQ: %d, %d byte data\n", send_packet.seq, f_len);
			
			
			if(f_len < 100) { // if i read file EOF , then stop server
				printf("%s sent (%d Bytes)\n",filename,total_len);
				send_packet.buf[f_len]=0;
				write(clnt_sock,(PACKET*) &send_packet, sizeof(PACKET));
				close(fd);
				close(clnt_sock); close(serv_sock);
				break;
			}
			write(clnt_sock,(PACKET*) &send_packet,sizeof(PACKET));

			
		}
		else { // in this hw , we don't do anything
		}



	}


	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

