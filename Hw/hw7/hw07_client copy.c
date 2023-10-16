#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>


#define BUF_SIZE 2048

#define SENDER 1
#define RECEIVER 2


typedef struct {
    int cmd;
    int buf_len;
    char buf[BUF_SIZE];
} PACKET;


void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	
	struct sockaddr_in serv_adr;
    struct timeval timeout;
    int mode;
    fd_set reads, cpy_reads;
    int fd_max, fd_num, i;

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));


	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));


    printf("-----------------------------------\n");
    printf("Choose function\n");
    printf("1. Sender,      2. Receiver\n");
    printf("=> ");
    scanf("%d",&mode);

	// Here Eorror Handleing

    if(mode == SENDER) {
        printf("File Sender Start!!\n");

    }
    else if(mode == RECEIVER) {
        printf("File Receiver Start!!\n");
    }

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

	FD_ZERO(&reads);
    int fd1, fd2;

	if(mode == SENDER) {
		if((fd1= open("./rfc1180.txt",O_RDONLY)) == -1) {
            printf("file open");
            exit(1);    
        }
        FD_SET(fd1, &reads);
		if(fd_max<fd1)
			fd_max=fd1;
        
        fd2= sock;
        FD_SET(fd2, &reads);
        if(fd_max<fd2)
			fd_max=fd2;
		
		printf("fd1: %d, fd2: %d\n",fd1, fd2);
		printf("max_fd: %d\n", fd_max);
	}
	else if(mode == RECEIVER) {
		fd1= sock;
        FD_SET(fd1, &reads);
        if(fd_max<fd1)
			fd_max=fd1;
		
		printf("fd1: %d\n", fd1);
		printf("max_fd: %d\n", fd_max);

	}

	
	while(1)
	{
		cpy_reads=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=5000;

		if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1) 
			break;

        printf("select!!");
		
		if(fd_num==0)
			continue;

		for(i=0; i<fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i==sock)     // connection request!
				{
			
				}
				else    // read message!
				{
				
				}
			}
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