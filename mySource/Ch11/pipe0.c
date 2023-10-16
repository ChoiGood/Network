#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    int fds[2];
    char str[] = "Self pipe test";
    char str2[] = "Self pipe test2";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);      // pipe creation
    printf("fds[0] = %d, fds[1] = %d\n", fds[0], fds[1]);

    // Writing through fds[1]
    write(fds[1], str, sizeof(str));
    write(fds[1], str2, sizeof(str2));
    //write(fds[1], str, sizeof(str));

    // Reading through fds[0]
    read(fds[0], buf, BUF_SIZE);
    puts(buf);

    read(fds[0], buf, BUF_SIZE);
    puts(buf);

    return 0;
}