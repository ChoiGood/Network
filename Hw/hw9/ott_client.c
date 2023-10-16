// Student Number : 2018110115      Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

// Buffer size
#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000
#define MAX_SIZE PREMIUM_BUF

#define FILE_REQ 11
#define FILE_SENDING 12
#define FILE_END 13
#define FILE_END_ACK 14

typedef struct {
int command;
int type;
char buf[MAX_SIZE];
int len;
} PACKET;

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

int x;
int max_buf;

int main(int argc, char * argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void * thread_return;

    int y;

    if(argc != 3) {
        printf("Usage : %s <IP> <port> \n", argv[0]);
        exit(1);
    }

    while(1)
    {
        printf("--------------------------------------------\n");
        printf("                 K-OTT Service              \n");
        printf("--------------------------------------------\n");

        printf("  Choose a subscribe type\n");
        printf("--------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3: Premium, 4: quit: ");
        
        scanf("%d", &x);

        if(x == 4) {
            printf("Exit program");
            exit(1);
        }
        else if(x > 4) {
            printf("Wong Command\n");
            continue;
        }

        printf("--------------------------------------------\n");
        printf("1. Download, 2: Back to Main menu: ");
        scanf("%d", &y);

        if(y == 1) {
          
            break;
        }
        else if (y == 2) {
            continue;
        }
        else {
            printf("Wonrg Command\n");
        }

    }


    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}



void * recv_msg(void * arg)     // read thread main
{
    int sock = *((int*)arg);

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));
    
    int rx_len;
    int tx_byte = 0;
    unsigned long t1, t2;
    unsigned long nano = 1000000000;
    struct timespec start, end;

    send_packet.type = x;

    if(x == 1) {
        max_buf = BASIC_BUF;
    } else if (x==2) {
        max_buf = STANDARD_BUF;
    }
    else if (x==3) {
        max_buf = PREMIUM_BUF;
    }

    // Send FIEL_REQ
    send_packet.command = FILE_REQ;
    write(sock, (void *) &send_packet, sizeof(PACKET)); 
    memset(&send_packet,0,sizeof(send_packet));
   

    clock_gettime(CLOCK_REALTIME, &start);
    t1 = start.tv_nsec + start.tv_sec * nano;

    while(1)
    {
        rx_len = read(sock, (void *) &recv_packet, sizeof(PACKET));
        //printf("%d\n", rx_len);
        //tx_byte += recv_packet.len;
        //printf("%d\n",recv_packet.command);

        if(recv_packet.command == FILE_SENDING) {
            tx_byte += recv_packet.len;
            //printf("%d\n", recv_packet.len);
            printf(".");
        }
        else if(recv_packet.command == FILE_END) {
            tx_byte += recv_packet.len;
            //printf("%d\n", recv_packet.len);
            printf(".\n");

            send_packet.command = FILE_END_ACK;
            write(sock, (void *) &send_packet, sizeof(PACKET)); 
            memset(&send_packet,0,sizeof(send_packet));

            break;
        }
        else {
            tx_byte += max_buf;
        }

        memset(&recv_packet,0,sizeof(recv_packet));
    }

    clock_gettime(CLOCK_REALTIME, &end);
    t2 = end.tv_nsec + end.tv_sec * nano;
    
    printf("File Transmission Finished\n");
    printf("Total received bytes: %d\n", tx_byte);
    printf("Downloading time: %ld msec\n", (t2-t1)/1000000);
    printf("Client closed\n");


    return NULL;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}