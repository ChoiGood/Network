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



#define BUF_SIZE 100
#define MAX_CLNT 256
void * handle_clnt(void * arg);

void error_handling(char * msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

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

    while(1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);

        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

    }
    close(serv_sock);
    return 0;
}

void * handle_clnt(void * arg)
{
    int clnt_sock = *((int*)arg);

    int rx_len = 0, i;

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

    int fd1;
    
    int max_buf;

    int tx_byte = 0;

    while(1)
    {
        rx_len = read(clnt_sock, (void *) &recv_packet, sizeof(PACKET));

        if(recv_packet.command == FILE_REQ) 
        {
            fd1 = open("hw09.mp4", O_RDONLY);
            

            // #undef MAX_SIZE
            // #define MAX_SIZE recv_packet.type

            if(recv_packet.type == 1) {
                max_buf = BASIC_BUF;
            }
            else if (recv_packet.type == 2) {
                max_buf = STANDARD_BUF;
            }
            else if (recv_packet.type == 3) {
                max_buf = PREMIUM_BUF;
            }
            
            //sleep(1);
            

            while(1)
            {
               
                send_packet.len = read(fd1, send_packet.buf, max_buf);
                //send_packet.buf[send_packet.len - 1] = 0;
                //printf("%d\n", send_packet.len);
                //tx_byte += send_packet.len;

                if(send_packet.len < max_buf) // file End
                {

                    pthread_mutex_lock(&mutx);
                    tx_byte += send_packet.len;
                

                    send_packet.command = FILE_END;
                    
                    write(clnt_sock, (void*) &send_packet, sizeof(PACKET));
                    
                    memset(&send_packet,0,sizeof(send_packet));
    
                    pthread_mutex_unlock(&mutx);

                    close(fd1);

                    break;
                }
                else // file not end
                {

                    pthread_mutex_lock(&mutx);
                    tx_byte += send_packet.len;

                    send_packet.command = FILE_SENDING;

                    write(clnt_sock, (void*) &send_packet, sizeof(PACKET));
                
                    memset(&send_packet,0,sizeof(send_packet));

                    pthread_mutex_unlock(&mutx);
                }
              
            }

            if(max_buf == BASIC_BUF) {
                printf("Total Tx Bytes: %d to Client %d (Basic)\n", tx_byte, clnt_sock);
            }
            else if(max_buf == STANDARD_BUF) {
                printf("Total Tx Bytes: %d to Client %d (Standard)\n", tx_byte, clnt_sock);
            }
            else if(max_buf == PREMIUM_BUF) {
                printf("Total Tx Bytes: %d to Client %d (Premium)\n", tx_byte, clnt_sock);
            }
            else {
                printf("Wrong buf size \n");
            }
            

        }
        else if (recv_packet.command == FILE_END_ACK)
        {
            printf("[Rx] FILE_END_ACK from Client %d => clnt_sock: %d closed.\n\n", clnt_sock, clnt_sock);
            break;
        }
        else
        {
            printf("Wrong Command receivved... %d \n",clnt_sock);
            break;
        }

        memset(&recv_packet,0,sizeof(recv_packet));
    }


    pthread_mutex_lock(&mutx);
    for(i=0; i<clnt_cnt; i++)   // remove disconnected client
    {
        if(clnt_sock == clnt_socks[i])
        {
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