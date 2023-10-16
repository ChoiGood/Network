#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
//cmd type
#define FILE_REQ 0
#define FILE_RES 1
#define FILE_END 2
#define FILE_END_ACK 3
#define FILE_NOT_FOUND 4

typedef struct {
    int cmd;
    int buf_len;
    char buf[BUF_SIZE];
} PACKET;


void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;

    PACKET recv_packet;
	PACKET send_packet;
	memset(&recv_packet,0,sizeof(recv_packet));
	memset(&send_packet,0,sizeof(send_packet));

	socklen_t clnt_adr_sz;
	struct sockaddr_in serv_adr, clnt_adr;

	if(argc!= 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

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

    FILE* f;
    char filename[BUF_SIZE];
    int count = 0; int total = 0;
	while(1)
	{
        recvfrom(serv_sock, &recv_packet, sizeof(recv_packet),0,(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        
        if(recv_packet.cmd == FILE_REQ) {
            printf("[Rx] cmd : %d, file_name: %s \n",recv_packet.cmd, recv_packet.buf);
            
           if((f= fopen(recv_packet.buf,"r")) == NULL) {  // file not exist
                send_packet.cmd = FILE_NOT_FOUND;
                printf("[Tx] cmd : %d, %s: File Not Fount\n", send_packet.cmd, recv_packet.buf);
                sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
				memset(&send_packet, 0, sizeof(send_packet));
				memset(&recv_packet, 0, sizeof(recv_packet));

                break;
		
           }
           else {  
            // file exist!! Then we have to send requsted FILE to Client
            while(1)
            {
                
                send_packet.buf_len = fread(send_packet.buf, sizeof(char), BUF_SIZE, f);
                printf("%d \n", send_packet.buf_len);
            
                if(send_packet.buf_len >= BUF_SIZE) {
                    send_packet.cmd =  FILE_RES;
                    count++; total += send_packet.buf_len;
                    
                    printf("[Tx] cmd : %d, len: %d, total_tx_cnt: %d, total_tx_bytes : %d\n",send_packet.cmd, send_packet.buf_len, count, total);
                    
                    sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
			        memset(&send_packet, 0, sizeof(send_packet));
                }
                else {
                    send_packet.cmd = FILE_END;
                    count++; total += send_packet.buf_len;
                    
                    printf("[Tx] cmd : %d, len: %d, total_tx_cnt: %d, total_tx_bytes : %d\n",send_packet.cmd, send_packet.buf_len, count, total);
                    
                    sendto(serv_sock, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
			        memset(&send_packet, 0, sizeof(send_packet));
                    memset(&recv_packet, 0, sizeof(recv_packet));
                    fclose(f);
                    break;
                }
                //sleep(1);
            }
            
           }          
        }
        else if(recv_packet.cmd == FILE_END_ACK) {
            printf("[Rx] cmd : %d, FILE_END_ACK\n", recv_packet.cmd);
            
            memset(&send_packet, 0, sizeof(send_packet));
            memset(&recv_packet, 0, sizeof(recv_packet));
                    
            break;
        }
        else {
            printf("recv_packet.cmd Not Match protocol\n");
            exit(1);
        }
		
	}

    printf("-----------------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n",count, total);
    printf("UDP Server Socket Close!\n");
    printf("-----------------------------------------------\n");
	close(serv_sock);
	return 0;

}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
