#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	pid_t pid = fork();
	if(pid == 0)  // if Child Process
	{
		puts("hi  i am a child process");
	}
	else
	{
		printf("child process ID : %d\n", pid);
		sleep(30); // sleep 30 sec.
	}

	if(pid==0)
		puts("End Child process");
	else
		puts("End parent process");

	return 0;
}
