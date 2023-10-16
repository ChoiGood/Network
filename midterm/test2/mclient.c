#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
//cmd type
#define FILE_REQ 0
#define FILE_RES 1
#define FILE_END 2
#define FILE_END_ACK 3
#define FILE_NOT_FOUND 4

typedef struct {
    int cmd;
    int buf_len;
    char buf[BUF_SIZE];
} PACKET;

void error_handling (char *message);

int main(int argc, char *argv[])
{
	int sock;

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

	socklen_t adr_sz;
	struct sockaddr_in serv_adr, from_adr;
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0 );
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0 , sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

    adr_sz = sizeof(from_adr);
    char file_name[BUF_SIZE];
    char fname[BUF_SIZE];

    /*
    printf("Input file name: ");
    fgets(file_name, BUF_SIZE, stdin);
    strncpy(fname, file_name , strlen(file_name)-1);
    */
    printf("Input file name: ");
	scanf("%s",file_name);

    send_packet.cmd = FILE_REQ;
    //send_packet.buf_len = strlen(fname);
    //strncpy(send_packet.buf, fname, send_packet.buf_len);
    strcpy(send_packet.buf,file_name);
    printf("[Tx] cmd: %d, file name: %s\n",send_packet.cmd, send_packet.buf);
    sendto(sock, &send_packet, sizeof(send_packet),0, (struct sockaddr*)&serv_adr , sizeof(serv_adr));
	memset(&send_packet,0, sizeof(send_packet));
		
    int count = 0; int total = 0;    

    while(1)
    {
        recvfrom(sock,  &recv_packet, sizeof(recv_packet),0, (struct sockaddr*) &from_adr, &adr_sz);

        if(recv_packet.cmd == FILE_RES)
        {
            printf("%s", recv_packet.buf);
            count ++; total += recv_packet.buf_len;
            memset(&recv_packet,0,sizeof(recv_packet));

        }
        else if (recv_packet.cmd == FILE_END)
        {
            printf("%s", recv_packet.buf);
            count ++; total += recv_packet.buf_len;
            memset(&recv_packet,0,sizeof(recv_packet));

            send_packet.cmd = FILE_END_ACK;

            printf("[Tx] cmd: %d, FILE_END_ACK\n",send_packet.cmd);
            sendto(sock, &send_packet, sizeof(send_packet),0, (struct sockaddr*)&serv_adr , sizeof(serv_adr));
	        memset(&send_packet,0, sizeof(send_packet));
            break;

        }
        else if (recv_packet.cmd == FILE_NOT_FOUND)
        {
            printf("[Rx] cmd: %d, %s: File Not Fount\n",recv_packet.cmd,file_name);
            
            break;
        }   
        else 
        {
            printf("recv packet.cmd Not match protocol\n");
            exit(1);
        }
        
    }
	
    printf("-----------------------------------------------\n");
    printf("Total Rx count: %d, bytes: %d\n",count, total);
    printf("UDP Client Socket Close!\n");
    printf("-----------------------------------------------\n");


	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
