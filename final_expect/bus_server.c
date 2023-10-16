// Student Number : 2018110115      Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

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



#define MAX_CLNT 256
void * handle_clnt(void * arg);
void error_handling(char * msg);
void PrintBus(int a[][COLS]);
int ReserveProcess(int num, int clnt);
int CancelProcess(int num, int clnt);
void CopyBus(int src[][COLS], int dest[][COLS]);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int bus[ROWS][COLS] = {0,};


int main(int argc, char * argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;

    if(argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("-----------------------------------------------------\n");
    printf("Bus Reservation System\n");
    printf("-----------------------------------------------------\n");

    while(1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);

        printf("Connected client IP: %s , clnt_sock=%d \n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);

    }
    close(serv_sock);
    return 0;
}

void * handle_clnt(void * arg)
{
    int clnt_sock = *((int*)arg);

    int rx_len = 0, i;

    REQ_PACKET recv_packet;
	RES_PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

  

    while(1)
    {
        rx_len = read(clnt_sock, (void *) &recv_packet, sizeof(recv_packet));

        if(recv_packet.command == INQUIRE)
        {
            send_packet.command = recv_packet.command;
            send_packet.seatno = recv_packet.seatno;
            send_packet.result = SUCCESS;
            CopyBus(bus, send_packet.seats);

            write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
            memset(&send_packet,0,sizeof(send_packet));
            
        }
        else if(recv_packet.command == RESERVE)
        {
            send_packet.command = recv_packet.command;
            send_packet.seatno = recv_packet.seatno;
            // reserve process --> result
            send_packet.result = ReserveProcess(recv_packet.seatno, clnt_sock);
            CopyBus(bus, send_packet.seats);

            write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
            memset(&send_packet,0,sizeof(send_packet));
        }
        else if(recv_packet.command == CANCEL)
        {
            send_packet.command = recv_packet.command;
            send_packet.seatno = recv_packet.seatno;
            // cancel process --> result
            send_packet.result = CancelProcess(recv_packet.seatno, clnt_sock);
            CopyBus(bus, send_packet.seats);

            write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
            memset(&send_packet,0,sizeof(send_packet));
        }
        else if(recv_packet.command == QUIT)
        {
            pthread_mutex_lock(&mutx);
            break;
        }
        else
        {
            printf("Receive Wrong packet command!!\n");
        }

        memset(&recv_packet,0,sizeof(recv_packet));
    }

   

    
    for(i=0; i<clnt_cnt; i++)   // remove disconnected client
    {
        if(clnt_sock == clnt_socks[i])
        {
            printf("Client removed: clnt_sock=%d, i=%d\n",clnt_sock,i);
            while(i < clnt_cnt)
            {
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);

    return NULL;
}

void error_handling(char * msg)
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

    return ;
}

int ReserveProcess(int num, int clnt)
{
    if( !(1<= num && num <= ROWS*COLS) )
    {
        return -1;
    }

    int n = num -1;
    
    int i = n / COLS;
    int j = n % COLS;

    pthread_mutex_lock(&mutx);
    if(bus[i][j] != 0) // bus already reserve ==> Reserve Error : Can't Reserve
    {
        pthread_mutex_unlock(&mutx);
        return -2;
    }

    
    bus[i][j] = clnt;

    pthread_mutex_unlock(&mutx);
    return 0;
}

int CancelProcess(int num, int clnt)
{
    if( !(1<= num && num <= ROWS*COLS) )
    {
        return -1;
    }

    int n = num -1;
    
    int i = n / COLS;
    int j = n % COLS;

    pthread_mutex_lock(&mutx);

    if(bus[i][j] == 0)
    {
        pthread_mutex_unlock(&mutx);
        return -3;
    }

    if(bus[i][j] != clnt)
    {
        pthread_mutex_unlock(&mutx);
        return -4;
    }

    bus[i][j] = 0;

    pthread_mutex_unlock(&mutx);
    return 0;
}

void CopyBus(int src[][COLS], int dest[][COLS])
{
    pthread_mutex_lock(&mutx);

    for(int i = 0; i<ROWS; i++) {
        for(int j=0; j<COLS; j++) {
            dest[i][j] = src[i][j];
        }
    }
     
    pthread_mutex_unlock(&mutx);

    return ;
}