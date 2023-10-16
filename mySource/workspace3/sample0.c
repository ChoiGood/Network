#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct
{
	int num;
	char name[10];
	float mid;
	float final;
	float total;
	double average;
}SCORE;

int main(void)
{
	int size = 5;
	SCORE* p = NULL;

	printf("sizeof(SCORE): %d, size: %d\n", (unsigned int) sizeof(SCORE), size);

	p = (SCORE*)malloc(size); // wrong code
	// p = (SCORE*)malloc(sizeof(SOCRE) * size); // right

	printf("allocated memory size: %d\n", (unsigned int) malloc_usable_size(p));

	if(p == NULL)
	{
		printf("memorry allocation error\n");
		exit(1);
	}
	
	free(p);
	return 0;
}
