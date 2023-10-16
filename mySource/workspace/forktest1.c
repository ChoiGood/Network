#include <stdio.h>
#include <unistd.h>

void forkexample()
{
	int x = 1;
	pid_t pid;

	pid = fork();
	if(pid == 0 )
		printf("Child has x= %d\n", ++x);
	else
		printf("Parent has x = %d, Child pid= %d\n", --x, pid);
}

int main()
{
	forkexample();
	return 0;
}
