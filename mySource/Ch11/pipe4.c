#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUF_SIZE 10

int main()
{
    int pipe_parent[2];
    int pipe_child[2];
    char wbuf[BUF_SIZE], rbuf[BUF_SIZE];
    int count = 100;
    int len = 0;
    pid_t pid;

    memset(wbuf, 0, sizeof(BUF_SIZE));
    memset(rbuf,0, sizeof(BUF_SIZE));

    if(pipe(pipe_parent) == -1)
    {
        printf("Parent pipe ceartion fail");
        exit(1);
    }

    if(pipe(pipe_child) == -1)
    {
        printf("Child pipe Creation Fail");
        exit(1);
    }

    pid = fork();

    if(pid == -1)
    {
        printf(" Child process Creation Fail");
        return -1;
    }

    if(pid == 0)
    {
        while(1)
        {
            count += 2;
            len = sprintf(wbuf, "%d", count);
            wbuf[len] = '\0';
            write(pipe_child[1], wbuf, strlen(wbuf));

            read(pipe_parent[0], rbuf, BUF_SIZE);
            sscanf(rbuf, "%d", &count);
            printf("<Child process> ount = %d\n", count);
            sleep(1);

        }
    }
    else
    {
        while(1)
        {
            read(pipe_child[0], rbuf, BUF_SIZE);
            sscanf(rbuf,"%d",&count);
            printf("[Parent process] count = %d\n", count);

            count -= 1;

            len = sprintf(wbuf, "%d", count);
            wbuf[len] = '\0';
            write(pipe_parent[1],wbuf,strlen(wbuf));
            sleep(1);
        }
    }

    return 0;
}
