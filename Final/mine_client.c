//Student Number : 2018110115     Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>


#define ROW 5
#define COL 5
#define MINE 999
#define CLNTA 50
#define CLNTB 30
#define MINEA 10
#define MINEB 5


#define RAND_START 1    // start random value
#define RAND_END 25		// end random value



#define CHECK 6

// cmd field value
#define GAME_READY 0 // Server -> Client
#define GAME_REQUEST 1 // Client -> Server
#define GAME_RESPONSE 2 // Server -> Client
#define GAME_END 3 // Server -> Client
// result field value
#define FAIL 0 // 해당 인덱스에 다른 클라이언트가 선택한 경우
#define SUCCESS 1 // 해당 인덱스를 선택하는 데 성공
#define MINE_FOUND 2 // 지뢰가 설치된 인덱스를 선택한 경우
// Client -> Server (GAME_REQUEST 메시지)
typedef struct {
int cmd; // cmd field value
int row; // 랜덤하게 선택한 row 값
int col; // 랜덤하게 선택한 col 값
}REQ_PACKET;
// Server -> Client (GAME_READY, GAME_RESPONSE, GAME_END 메시지)
typedef struct {
int cmd; // cmd field value
int board[ROW][COL]; // 클라이언트 선택 현황 (4: client #1, 5: client #2, -1)
int result; // result 값: FAIL, SUCCESS, MINE_FOUND, OUTOF_RANGE
}RES_PACKET;


void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
void PrintBoard(int a[][COL]);

static sem_t sem_one;
static sem_t sem_two;

int exit_flag = 0;

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

    sem_init(&sem_one,0,0);
    sem_init(&sem_two,0,1);

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    sem_destroy(&sem_one);
    sem_destroy(&sem_two);

    close(sock);
    return 0;
}

void * send_msg(void * arg) // send thread main
{
    int sock = *((int*)arg);
    
    REQ_PACKET send_packet;
	memset(&send_packet,0,sizeof(send_packet));

    srand(time(NULL));


    while(1)
    {
        sem_wait(&sem_one);

        send_packet.cmd = GAME_REQUEST;
        send_packet.row = (rand() % COL);
        send_packet.col = (rand() % COL);
        
        write(sock, (void *) &send_packet, sizeof(send_packet)); 
        memset(&send_packet,0,sizeof(send_packet));

        sem_post(&sem_two);
    }


    return NULL;
}

void * recv_msg(void * arg)     // read thread main
{
    int sock = *((int*)arg);
    
    RES_PACKET recv_packet;
    memset(&recv_packet,0,sizeof(recv_packet));

    int rx_len;

    while(1)
    {
        sem_wait(&sem_two);

        rx_len = read(sock, (void *) &recv_packet, sizeof(recv_packet));
        printf("[Rx] cmd : %d, result: %d\n", recv_packet.cmd, recv_packet.result);
        
        if(recv_packet.cmd == GAME_READY)
        {
            printf("GAME_READY received.\n");
            PrintBoard(recv_packet.board);
        }
        else if(recv_packet.cmd == GAME_RESPONSE)
        {
            if(recv_packet.result == MINE_FOUND) {
                printf("MINE_FOUND\n");
            }
            PrintBoard(recv_packet.board);
        }
        else if(recv_packet.cmd == GAME_END)
        {
            printf("END\n");
            PrintBoard(recv_packet.board);
            break;

        }

        memset(&recv_packet,0,sizeof(recv_packet));

        sleep(1);
        sem_post(&sem_one);
    }

    return NULL;
}


void PrintBoard(int a[][COL])
{
    printf("+------------------------+\n");
	for(int i=0; i<ROW;i++)
	{
		for(int j=0; j<COL; j++)
		{
			if(a[i][j] != 0) {
                
                    if(a[i][j] == MINEA) {
                        printf("|  O ");
                    }
                    else if(a[i][j] == MINEB) {
                        printf("|  X ");
                    }
                    else if(a[i][j] == CLNTA) {
                        printf("|  . ");
                        
                    }
                    else if(a[i][j] == CLNTB) {
                        printf("|  - ");
                        
                    }
                    else {
                        printf("|    ");
                    }
                
				
			} else {
				printf("|    ");
			}
		}
		printf("|\n");
		printf("+------------------------+\n");
	} 
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}



