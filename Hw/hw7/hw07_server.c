// Student Number : 2018110115 			Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048

#define SENDER 1
#define RECEIVER 2


typedef struct {
    int cmd;
    int buf_len;
    char buf[BUF_SIZE];
} PACKET;




void error_handling(char *buf);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max=serv_sock;


    // adr_sz=sizeof(clnt_adr);
    // clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);

    // str_len= read(clnt_sock, &recv_packet, sizeof(PACKET));

    // printf("%s", recv_packet.buf);

    

	while(1)
	{
		cpy_reads=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=5000;

		if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1) 
			break;

        //printf("select!!");
		
		if(fd_num==0)
			continue;

		for(i=0; i<fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i==serv_sock)     // connection request!
				{
					adr_sz=sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					FD_SET(clnt_sock, &reads);
					if(fd_max<clnt_sock)
						fd_max=clnt_sock;
					printf("connected client: %d \n", clnt_sock);
				}
				else    // read message!
				{
					str_len= read(i, &recv_packet, sizeof(PACKET));
                    
					if(str_len==0)    // close request!
					{
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d \n", i);
					}
					else
					{
                        
						//printf("%d\n", recv_packet.cmd);
						//printf("%s",recv_packet.buf);

						// ANOTHER CLNT SOCK FIND
						
                        int temp,j;
                        for(j=3; j<fd_max+1; j++) { 
                            if(j != i && j!=serv_sock) {
                                temp = j;
                            }
                        }
						//printf("%d\n", temp);
                        if(recv_packet.cmd == SENDER) { 
                            printf("FORWARD [%d] -----> [%d]\n",i,temp);
                            recv_packet.cmd = RECEIVER;
                        }
                        else if(recv_packet.cmd == RECEIVER) {
                            printf("BACKWARD [%d] <----- [%d]\n",temp,i);
                            recv_packet.cmd = SENDER;
                        }

                        // strcpy(send_packet.buf,recv_packet.buf);
                        // send_packet.buf_len = recv_packet.buf_len;
                        write(temp,(PACKET*) &recv_packet,sizeof(PACKET));

                        memset(&recv_packet,0,sizeof(recv_packet));
	                    //memset(&send_packet,0,sizeof(send_packet));
					}
				}
			}
		}
	}
    
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}