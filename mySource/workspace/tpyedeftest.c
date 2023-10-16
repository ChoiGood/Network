#include <stdio.h>

// function pointer ptrfunc definition
typedef void (*ptrfunc)(int,int);

void add(int a, int b)
{
	printf("a + b = %d\n", a + b);
}

void sub(int a, int b)
{
	printf("a - b = %d\n", a - b);
}

int main()
{
	ptrfunc handler;

	handler = add;
	handler(2,3);

	handler = sub;
	handler(10,4);

	return 0;
}
