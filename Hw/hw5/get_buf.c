#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;
	int snd_buf, rcv_buf, state;
	socklen_t len;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	len = sizeof(snd_buf);
	state = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, &len);
	if(state)
		error_handling("getsockopt() error");

	len = sizeof(rcv_buf);
	state = getsockopt(sock , SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, &len);
	if(state)
		error_handling("getsockopt() error");

	printf("Receive buffer size: %d \n", rcv_buf);
	printf("Send buffer size : %d \n" , snd_buf);

	
	
	return 0;
}
