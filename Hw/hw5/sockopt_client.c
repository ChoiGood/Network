// Student Number : 2018110115      Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define REQ 2
#define SUCESS 0
#define FAIL -1


typedef struct {
   int level;
   int option;
   int optval;      // req_socket values
   int result;      // req_ socket optioin result (0: Sucess , -1 : Fail)
} SO_PACKET;

struct flaginfo {
    int number;
    int level;
    int opt;
    char* optname;
};


struct flaginfo socketopt[] = {
    1, SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF",
    2, SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF",
    3, SOL_SOCKET, SO_REUSEADDR, "SO_REUSEADDR",
    4, SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE",
    5, SOL_SOCKET, SO_BROADCAST, "SO_BROADCAST",
    6, IPPROTO_IP, IP_TOS, "IP_TOS",
    7, IPPROTO_IP, IP_TTL, "IP_TTL",
    8, IPPROTO_TCP, TCP_NODELAY, "TCP_NODELAY",
    9, IPPROTO_TCP, TCP_MAXSEG, "TCP_MAXSEG",
    0, 0, 0, NULL
};

void PrintOption()
{
    printf("------------------------------------------\n");
    int i;
    for(i=0; socketopt[i].number; i++) {
        printf("%d: %s\n",socketopt[i].number, socketopt[i].optname);
    }
    printf("%d: %s\n",i+1,"Quit");

    printf("------------------------------------------\n");
}

void error_handling (char *message);

int main(int argc, char *argv[])
{
	int sock;

    SO_PACKET recv_packet;
	SO_PACKET send_packet;
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
    int optnum = sizeof(socketopt) / sizeof(struct flaginfo);
    //printf("%d\n\n",optnum);
    int flag = 1;

    while(1)
    {
        int x;
        if(flag) PrintOption();
        printf("Input option number: ");
        scanf("%d",&x);

        if (x>=1 && x< optnum) {
            flag = 1;
            send_packet.result = REQ;
            send_packet.level = socketopt[x-1].level;
            send_packet.option = socketopt[x-1].opt;
            send_packet.optval = x;
            sendto(sock, &send_packet, sizeof(send_packet),0, (struct sockaddr*)&serv_adr , sizeof(serv_adr));
	        memset(&send_packet,0, sizeof(send_packet));

            recvfrom(sock,  &recv_packet, sizeof(recv_packet),0, (struct sockaddr*) &from_adr, &adr_sz);
            if(recv_packet.result == SUCESS) {
                printf(">>> Server result: %s: value: %d, result: %d\n\n",socketopt[x-1].optname,recv_packet.optval,recv_packet.result);

            }
            else if(recv_packet.result == FAIL) {
                printf(">>> Server result: %s: GETINFO FAIL... \n\n",socketopt[x-1].optname);
            }
            else {
                printf("recv_packet.result something wrong\n");
                exit(1);
            }
            
            memset(&recv_packet,0,sizeof(recv_packet));

        }
        else if (x== optnum) {
            printf("Client quit.\n");
            break;
        }
        else {
            flag = 0;
            printf("Wrong number. type again!\n");
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
