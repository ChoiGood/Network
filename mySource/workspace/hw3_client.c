// Student Number : 2018110115 		Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>


#define BUF_SIZE 100
#define SEQ_START 1000

#define FILEREQUEST 50
#define FILENAMELENMAX 100
#define FILENOT 1


typedef struct {
	int seq;				// SEQ number
	int ack;				// ACK number
	int buf_len;			// File read/write bytes
	char buf[BUF_SIZE];		// file name or file contents
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

	int fd;
	char filename[FILENAMELENMAX];
	int total =0;
	int f_len;

	printf("Input file name: ");
	scanf("%s",filename);

	send_packet.seq=FILEREQUEST;
	strcpy(send_packet.buf,filename);
	printf("[Client] request %s\n\n", send_packet.buf);
	write(sock,(PACKET*) &send_packet,sizeof(PACKET));
	

	rx_len = read(sock, &recv_packet, sizeof(PACKET));
	recv_packet.buf[BUF_SIZE]=0;

	if(rx_len == -1)
		error_handling("read() error!!");

	if(recv_packet.seq == FILENOT) {
		printf("%s\n",recv_packet.buf);
		exit(1);
	}
	else if(recv_packet.seq == SEQ_START) {
		fd = open(send_packet.buf, O_CREAT|O_WRONLY|O_TRUNC, 0664);    // file creat and open!!
		
		f_len = strlen(recv_packet.buf);
		
		write(fd, recv_packet.buf,f_len);   // write file contents!!
		total += f_len;

		printf("[Client] Rx SEQ: %d, len: %d bytes\n",recv_packet.seq,f_len);
	
		if(f_len < 100) {  // Exit Condition!!  if File end, then we stop
			printf("%s received (%d Bytes)\n",send_packet.buf,total);
			close(fd);
			exit(1);
		}
		else {
			send_packet.ack = recv_packet.seq + BUF_SIZE;
			printf("[Client] Tx ACK: %d\n\n",send_packet.ack);
			write(sock,(PACKET*) &send_packet,sizeof(PACKET));
		}
	}
	else {
		error_handling("other data recv..");
	}

	while(1) {
		rx_len = read(sock, &recv_packet, sizeof(PACKET));
		if(rx_len == -1)
			error_handling("read() error!");

		if(send_packet.ack == recv_packet.seq) {   // previouse sned ack == recv_seq   then  sned new ack!!
			f_len = strlen(recv_packet.buf);
			write(fd, recv_packet.buf,f_len);  total += f_len;     //file write and total byte store
			
			printf("[Client] Rx SEQ: %d, len: %d bytes\n",recv_packet.seq,f_len);

			if(f_len < 100) {   // receive FILE END 
				printf("%s received (%d Bytes)\n",send_packet.buf, total);
				break;
			}
			else {		// FILE NOT END ,, SO we recive more
				send_packet.ack = recv_packet.seq + BUF_SIZE;
				printf("[Client] Tx ACK: %d\n\n",send_packet.ack);
				write(sock,(PACKET*) &send_packet,sizeof(PACKET));
			}

		}
		else {
			
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

