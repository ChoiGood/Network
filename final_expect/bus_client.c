// Student Number : 2018110115      Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

// int command
#define INQUIRE 1
#define RESERVE 2
#define CANCEL 3
#define QUIT 4

// result
#define SUCCESS 0   
#define WSN -1
#define RESERVEFAIL -2
#define CANCELFAIL -3
#define CANCELFAIL2 -4

#define ROWS 2
#define COLS 10

// Response Packet (Server -> Client)
typedef struct {
    int command;
    int seatno;
    int seats[ROWS][COLS];
    int result;
} RES_PACKET;


// Request Packet(Client -> Server)
typedef struct {
    int command;
    int seatno;
} REQ_PACKET;

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
void PrintBus(int a[][COLS]);

int main(int argc, char * argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void * thread_return;

    

    if(argc != 3) {
        printf("Usage : %s <IP> <port> \n", argv[0]);
        exit(1);
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

    RES_PACKET recv_packet;
	REQ_PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));
    
    int rx_len;
    int x;
    int num;

    while(1)
    {
        printf("1: inquiry, 2: reservation, 3: cancellation, 4: quit: ");
        scanf("%d",&x);

        if(x==1)
        {
            send_packet.command = x;
            write(sock, (void *) &send_packet, sizeof(send_packet)); 
            memset(&send_packet,0,sizeof(send_packet));
   
        }
        else if (x==2)
        {
            printf("Input seat number: ");
            scanf("%d",&num);

            send_packet.command = x;
            send_packet.seatno = num;
            write(sock, (void *) &send_packet, sizeof(send_packet)); 
            memset(&send_packet,0,sizeof(send_packet));
        }
        else if (x==3)
        {
            printf("Input seat number for cancellation: ");
            scanf("%d", &num);
            
            send_packet.command = x;
            send_packet.seatno = num;
            write(sock, (void *) &send_packet, sizeof(send_packet)); 
            memset(&send_packet,0,sizeof(send_packet));

        }
        else if (x==4)
        {
            send_packet.command = x;
            write(sock, (void *) &send_packet, sizeof(send_packet)); 
            memset(&send_packet,0,sizeof(send_packet));

            printf("Quit\n");
            break;
        }

        rx_len = read(sock, (void *) &recv_packet, sizeof(recv_packet));

        if(recv_packet.result == SUCCESS)
        {
            PrintBus(recv_packet.seats);
            printf("Operation suceess.\n");
        }
        else if(recv_packet.result == WSN)
        {
            PrintBus(recv_packet.seats);
            printf("Wrong seat number.\n");
        }
        else if(recv_packet.result == RESERVEFAIL)
        {
            PrintBus(recv_packet.seats);
            printf("Reservation failed.  (The seat was not already reserved.)\n");
        }
        else if(recv_packet.result == CANCELFAIL)
        {
            PrintBus(recv_packet.seats);
            printf("Cancellation failed.  (The seat was not reserved.)\n");
        }
        else if(recv_packet.result == CANCELFAIL2)
        {
            PrintBus(recv_packet.seats);
            printf("Cancellation failed.  (The seat was reserved by another person.)\n");
        }
        else
        {
            printf("Client Receive Wrong packet command\n");
        }
        
    }


    return NULL;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


void PrintBus(int a[][COLS])
{
    for(int i = 0; i<ROWS; i++) {
        printf("---------------------------------------------------\n");
        for(int j = 1; j<= COLS; j++) {
            printf("|%3d ",10*i + j);
        }
        printf("|\n");
        printf("---------------------------------------------------\n");


        for(int j = 0; j<COLS; j++) {
            printf("|%3d ",a[i][j]);
        }
        printf("|\n");
    }
}
