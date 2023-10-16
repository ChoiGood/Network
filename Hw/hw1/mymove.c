// 2018110115 Ji Won Choi
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 10

void error_handling(char* message);

int main(int argc, char* argv[])
{
	int fd1,fd2;
	int total=0;
	int size;
	char buf[BUF_SIZE];
	

	if (argc != 3){
		printf("[Error] mymove Usage: %s src_file dest_file\n",argv[0]);
		exit(1);
	}

	fd1 = open(argv[1], O_RDONLY);
	if(fd1==-1)
		error_handling("source file open() error!");
	fd2 = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC,0644);
	if(fd2==-1)
		error_handling("destination file open() error!");


	while(size=read(fd1,buf,BUF_SIZE)) {
			if(size==-1)
				error_handling("read() error!");
			write(fd2,buf,size);
			total+=size;
	}

	close(fd1);
	close(fd2);

	printf("move from %s to %s (bytes: %d) finished.\n",argv[1],argv[2],total);
	remove(argv[1]);
	
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
