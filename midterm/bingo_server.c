// Student Number : 2018110115 				Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define ROW 4
#define COL 4
#define CHECK ROW*COL

#define RAND_START 1    // start random value
#define RAND_END 30		// end random value

// result field value
#define FAIL 0
#define SUCCESS 1
#define CHECKED 2

// cmd field value
#define BINGO_REQ 0
#define BINGO_RES 1
#define BINGO_END 2

// Client -> Server 
typedef struct {
	int cmd;		//BINGO_REQ
	int number;		// RANDOM created number (1~30)
}REQ_PACKET;

// Server -> Client
typedef struct {
	int cmd;				// BINGO_RES, BINGO_END
	int number;				// Number choosed by User
	int board[ROW][COL];	// bingo correct board (0 or bingo number)
	int result;				// result from the server(FAIL, SUCCESS, CHECKED)
}RES_PACKET;

int bingo_array[ROW][COL] = {0, };				// random bingo number array(1 ~ 30 number stored)
int player_choice_array[ROW][COL] = {0, };		// Client correct number board

void error_handling(char *message);

void CreatBingo()
{
	int bingo_check[CHECK] = {0,};
	int count=0;
	int rand_num , flag;
	srand(time(NULL));

	while(1)
	{
		rand_num = (rand() % RAND_END) + RAND_START;
		
		flag =1;
		for(int i=0; i<count; i++) {  // check the overlap number
			if(rand_num == bingo_check[i])
				flag = 0;
		}

		if(flag) {  // new number
			bingo_array[count/COL][count%COL] = rand_num;
			bingo_check[count] = rand_num;
			count++;
		}
			
		if(count ==( ROW * COL)) break;
	}
}

void PrintBingo(int a[][COL])
{
	printf("+-------------------+\n");
	for(int i=0; i<ROW;i++)
	{
		for(int j=0; j<COL; j++)
		{
			if(a[i][j] != 0) {
				printf("|%3d ",a[i][j]);
			} else {
				printf("|    ");
			}
		}
		printf("|\n");
		printf("+-------------------+\n");
	}
}

// b <- a
void CopyBingo(int a[][COL], int b[][COL])
{
	for(int i=0; i<ROW; i++ ) {
		for(int j=0; j<COL; j++) {
			b[i][j] = a[i][j];
		}
	}
}

int main(int argc, char* argv[])
{
	CreatBingo();
	PrintBingo(bingo_array);
	printf("\nReaady!\n");

	int serv_sock;
	REQ_PACKET recv_packet;
	RES_PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

	//...int pac_len;
	socklen_t clnt_adr_sz;
	struct sockaddr_in serv_adr, clnt_adr;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1)
		error_handling("UDP socket creation error");

	memset(&serv_adr, 0 , sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	clnt_adr_sz = sizeof(clnt_adr);
	
	int cnt=0;

	while(1)
	{
		// receive the Packet from client
		recvfrom(serv_sock,&recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);  
		printf("[Rx] BINGO_REQ(cmd: %d, number: %d)\n", recv_packet.cmd, recv_packet.number);

		if(recv_packet.cmd == BINGO_REQ)
		{
			send_packet.number = recv_packet.number; // number stored

			// we decide Result!! by received Number
			
			int flag1 =0;   // Check number in bingo array
			int row, col;
			for(int i=0; i<ROW; i++) {
				for(int j=0; j<COL; j++) {
					if(recv_packet.number == bingo_array[i][j]) {
						flag1 = 1 , row = i, col = j;
					}
				}
			}
			
			if (flag1) { // Ok in bingo
				int flag2 = 0; // Check number in User already  chosed array
				for(int i=0; i<ROW; i++) {
					for(int j=0; j<COL; j++) {
						if(recv_packet.number == player_choice_array[i][j])
							flag2=1;
					}
				}

				if(flag2) { // number already in User Array... So we can't change anything..
					send_packet.result = CHECKED;
					printf("Client already chose(num: %d)\n",recv_packet.number);
				}
				else { // Number also Not in User Array ==> we can Add Number UserArray!!
					send_packet.result = SUCCESS;
					player_choice_array[row][col] = recv_packet.number;//Add number in  UserArray 
					cnt++;
					printf("Bingo: [%d][%d]: %d \n",row,col,recv_packet.number);
				}
				
			}
			else { // Not in bingo
				send_packet.result = FAIL;
				printf("Not found: num: %d\n",recv_packet.number);
			}
			//-------------------------------- result decided And Action according to Result -----------------------------
			

			// store board at send_packet.board
			CopyBingo(player_choice_array,send_packet.board); 

			// decide CMD
			if(cnt == CHECK) { 
				send_packet.cmd = BINGO_END;
				printf("No available space.\n");
				printf("[Tx] BINGO_END\n");
				printf("Exit Server\n");
				
				sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
				memset(&send_packet, 0, sizeof(send_packet));
				memset(&recv_packet, 0, sizeof(recv_packet));

				break;
			}
			else {
				send_packet.cmd = BINGO_RES;
			}
			
			printf("[Tx] BINGO_RES(cmd: %d, result: %d)\n", send_packet.cmd, send_packet.result);
			// print User bingo
			PrintBingo(player_choice_array);
			printf("\n");
			
			// Send Packet to client
			sendto(serv_sock,&send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
			memset(&send_packet, 0, sizeof(send_packet));
			memset(&recv_packet, 0, sizeof(recv_packet));


		}
		else {
			printf("recv_packet.cmd wrong value...\n");
		}
		

		//CopyBingo(bingo_array,send_packet.board);

		//sendto(serv_sock,&send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
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



// Professor review 
/*
   Use function please... 
   exist_num(num)
  int  check_bingo_number(INT NUM)

  fuction Make...


  !! memcpy == > board copy !! USe it 
*/
