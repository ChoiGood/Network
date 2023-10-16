// Student Number : 2018110115              Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define TTL 64
#define BUF_SIZE 120
#define TRUE 1
#define FALSE 0
void error_handling(char *message);

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <GroupIP> <PORT> <NAME>\n", argv[0]);
        exit(1);
    }

    pid_t pid = fork();




    if(pid == 0) // child process ===> Multicast Receiver
    {
        int recv_sock;
        int str_len;
        char buf[BUF_SIZE];
        struct sockaddr_in adr;
        struct ip_mreq join_adr;

        int option;
        socklen_t optlen;

        recv_sock = socket(PF_INET, SOCK_DGRAM, 0);

        optlen = sizeof(option);
        option = TRUE;

        setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);


        memset(&adr, 0, sizeof(adr));
        adr.sin_family = AF_INET;
        adr.sin_addr.s_addr = htonl(INADDR_ANY);
        adr.sin_port = htons(atoi(argv[2]));

        if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
            error_handling("bind() error");
        
        join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
        join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

        setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));


        while(1)
        {
            str_len = recvfrom(recv_sock, buf, BUF_SIZE-1, 0, NULL, 0);
            if(str_len < 0)
                break;
            
            buf[str_len] = 0;
            fputs(buf, stdout);
        }
        close(recv_sock);
        return 0;
    }
    else if (pid > 0)  // Parent Process ===> Multicast Sender
    {
        int send_sock;
        struct sockaddr_in mul_adr;
        int time_live = TTL;
        
        char buf[BUF_SIZE];

        char name[20]= "[ji won choi]";
        char msg[100];

        sprintf(name, "[%s]", argv[3]);

        send_sock = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&mul_adr, 0, sizeof(mul_adr));
        mul_adr.sin_family = AF_INET;
        mul_adr.sin_addr.s_addr = inet_addr(argv[1]); // Multicast IP
        mul_adr.sin_port = htons(atoi(argv[2])); // Multicast Port

        setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &time_live, sizeof(time_live));

        while(1)
        {
            fgets(msg, sizeof(msg), stdin);
            if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
            {
                kill(pid, SIGKILL);
                printf("SIGKILL: Multicast Receiver terminate!\n");
                break;
            }
            sprintf(buf,"%s %s", name, msg);
            sendto(send_sock, buf, strlen(buf),0,(struct sockaddr*) &mul_adr, sizeof(mul_adr));

        }

        printf("Multicast Sender(Parent process) exit\n");
        close(send_sock);
        return 0;
    }
    else
    {
        error_handling("fork");
    }
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}