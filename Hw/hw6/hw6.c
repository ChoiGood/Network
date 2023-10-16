// Student Number : 2018110115          Student Name : Ji Won Choi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void timeout(int sig)
{
    int static total = 0;
    total += 2;
    if(sig==SIGALRM)
        printf("<Parent> time out: 2, elapsed tiem: %4d seconds\n", total);
    
    alarm(2);
}

int count = 0;
void timeout2(int sig)
{
    int static total =0;

    total += 5;
    count += 1;

    if(sig == SIGALRM)
        printf("[Child] time out: 5, elapsed time: %4d seconds(%d)\n", total, count);

    
    alarm(5);
}

void read_childproc(int sig)
{
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);
    if(WIFEXITED(status))
    {
        printf("Child id: %d, sent: %d\n", id, WEXITSTATUS(status));
    }
}

void decide_exit(int sig)
{
    char c;
    printf("SIGINT: Do you want to exit (y or Y to exit)? ");
    scanf("%c",&c);
    if(c == 'y' || c == 'Y')
        exit(1);
}

int main()
{
    pid_t pid;
    struct sigaction act1, act2, act3;          // act1 : SIGALRM   act2 : SIGCHILD  act3 : SIGINT

    pid = fork();

    if(pid == -1) {
        printf("fork() error\n");
        exit(1);
    }
    else if(pid == 0) {  // Child Process
        act1.sa_handler = timeout2;
        sigemptyset(&act1.sa_mask);
        act1.sa_flags = 0;
        sigaction(SIGALRM, &act1, 0);

        alarm(5);

        while(1) {
            if (count == 5) {
                break;
            }
        }

        exit(5);

    }
    else {   // Parent Process
        act1.sa_handler = timeout;
        sigemptyset(&act1.sa_mask);
        act1.sa_flags = 0;
        sigaction(SIGALRM, &act1, 0);

        act2.sa_handler = read_childproc;
        sigemptyset(&act2.sa_mask);
        act2.sa_flags = 0;
        sigaction(SIGCHLD, &act2, 0);

        act3.sa_handler = decide_exit;
        sigemptyset(&act3.sa_mask);
        act3.sa_flags = 0;
        sigaction(SIGINT, &act3, 0);

        alarm(2);       // 2 seconds timer in Parent Process

        while(1)        // Infinite Loop
        {
            sleep(1);
        }


    }


    return 0;
}