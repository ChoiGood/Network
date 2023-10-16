// Student Number : 2018110115          Student Name : Ji Won Choi

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

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock; // udp
    int tcp_sock;  // option value check
    socklen_t optlen;
    int state;

    SO_PACKET recv_packet;
	SO_PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

	socklen_t clnt_adr_sz;
	struct sockaddr_in serv_adr, clnt_adr;

	if(argc!= 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    tcp_sock = socket(PF_INET, SOCK_STREAM, 0);

	// UDP socket creation
	serv_sock = socket(PF_INET, SOCK_DGRAM,0);
	if(serv_sock==-1)
		error_handling("UDP socket creation error");

	memset(&serv_adr, 0 , sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

    clnt_adr_sz = sizeof(clnt_adr);

    printf("Socket Option Server Start\n\n\n\n\n\n\n\n\n");
    int x;
    while(1)
    {
        recvfrom(serv_sock, &recv_packet, sizeof(recv_packet),0,(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        x= recv_packet.optval;
        printf(">>> Received Socket option: %s\n", socketopt[x - 1].optname);
        
        if(recv_packet.result == REQ)
        {
            optlen = sizeof(recv_packet.optval);
            state = getsockopt(tcp_sock, recv_packet.level,recv_packet.option,(void*)&recv_packet.optval, &optlen );
            if(state) {
                send_packet.result = FAIL;
            }
            else {
                send_packet.result = SUCESS;
                send_packet.optval = recv_packet.optval;
            }
            printf("<<< Send option: %s: %d, result: %d\n\n\n\n\n\n\n\n\n\n\n\n\n\n",socketopt[x-1].optname, send_packet.optval, send_packet.result);
            sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
			memset(&send_packet, 0, sizeof(send_packet));

        }
        else
        {
            printf("Recieve Wrong value\n");
        }
        memset(&recv_packet, 0, sizeof(recv_packet));

    }

    close(serv_sock);
	return 0;

}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
