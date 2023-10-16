//Student Number : 2018110115     Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

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


#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int lifeA =3;
int lifeB =3;
int Bomb = CHECK;

int game_board[ROW][COL] = {0,};

void InitBoard(void);
void InitUserBoard(int a[][COL]);
void PrintBoard(int a[][COL]);
int ReqProcess(int row, int col, int clnt);
void CopyBoard(int src[][COL], int dest[][COL]);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

    InitBoard();
    PrintBoard(game_board);
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
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
	int clnt_sock=*((int*)arg);
	
    int first = 1;

    int rx_len = 0, i;

    REQ_PACKET recv_packet;
	RES_PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));


    while(1)
    {
        if(first)
        {
            send_packet.cmd = GAME_READY;
            send_packet.result = SUCCESS;
            InitUserBoard(send_packet.board);

            printf("cmd: 0, client_id : %d, result : 1\n", clnt_sock);

            write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
            memset(&send_packet,0,sizeof(send_packet));
            first = 0;
        }

        rx_len = read(clnt_sock, (void *) &recv_packet, sizeof(recv_packet));

        if(recv_packet.cmd == GAME_REQUEST)
        {   
            PrintBoard(game_board);
            send_packet.result = ReqProcess(recv_packet.row,recv_packet.col, clnt_sock);

            CopyBoard(game_board, send_packet.board);

            if(lifeA ==0 || lifeB == 0 || Bomb == 0) {
                send_packet.cmd = GAME_END;
                write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
                memset(&send_packet,0,sizeof(send_packet));

                printf("Client id %d died\n", clnt_sock);

                break;
            }

            send_packet.cmd = GAME_RESPONSE;

            write(clnt_sock, (void*) &send_packet, sizeof(send_packet));        
            memset(&send_packet,0,sizeof(send_packet));

        }

        memset(&recv_packet,0,sizeof(recv_packet));
    
    }





	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		printf("clnt_sock: %d, clnt_cnt: %d\n", clnt_sock, clnt_cnt);
		if(clnt_sock==clnt_socks[i])
		{
			while(i < clnt_cnt)
			{
				clnt_socks[i]=clnt_socks[i+1];
				i++; // 클라이언트 종료시점에 무한루프 발생 문제점 수정
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

void InitBoard(void)
{
    
    int board_check[CHECK] = {0,};
    int count = 0;
    int rand_num , flag;
    srand(time(NULL));

    
    while(1)
    {
        rand_num = (rand() % RAND_END) + RAND_START;
		flag =1;
        for(int i=0; i<count; i++) {  // check the overlap number
			if(rand_num == board_check[i])
				flag = 0;
		}

        if(flag) {  // new number
            int temp = rand_num -1;
			game_board[temp/COL][temp%COL] = MINE;
			board_check[count] = rand_num;
			count++;
		}


        if(count == (CHECK)) break;
    }
}

void InitUserBoard(int a[][COL])
{
    for(int i=0; i<ROW; i++)
    {
        for(int j=0; j<COL; j++)
        {
            a[i][j] = 0;
        }
    }
}

void PrintBoard(int a[][COL])
{
    printf("+------------------------+\n");
	for(int i=0; i<ROW;i++)
	{
		for(int j=0; j<COL; j++)
		{
			if(a[i][j] != 0) {
                if(a[i][j] == MINE) {
                    printf("|  B ");
                }
                else {
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
                }
				
			} else {
				printf("|    ");
			}
		}
		printf("|\n");
		printf("+------------------------+\n");
	} 
}

int ReqProcess(int row, int col, int clnt)
{
    int result;
    pthread_mutex_lock(&mutx);
    if(game_board[row][col] == 0) 
    {
        if(clnt == clnt_socks[0]) {
            game_board[row][col] = CLNTA;
        }
        else {
            game_board[row][col] = CLNTB;
        }

        result = 1;
    }
    else if(game_board[row][col] == MINE)
    {
        if(clnt == clnt_socks[0]) {
            game_board[row][col] = MINEA;
        }
        else {
            game_board[row][col] = MINEB;
        }
        Bomb--;

        // ADD
        result = 2;
    }
    else
    {
        printf("Another client already chose.(%d,%d)\n",row,col);
        if(clnt == clnt_socks[0]) {
            lifeA--;
        }
        else {
            lifeB--;
        }
        result = 0;
    }

    pthread_mutex_unlock(&mutx);

    return result;
}

void CopyBoard(int src[][COL], int dest[][COL])
{
    pthread_mutex_lock(&mutx);

    for(int i = 0; i<ROW; i++) {
        for(int j=0; j<COL; j++) {
            dest[i][j] = src[i][j];
        }
    }
     
    pthread_mutex_unlock(&mutx);

    return ;
}