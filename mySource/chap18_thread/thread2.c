#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void* thread_main(void *arg);
char msg[50] = {0};
int main(int argc, char* argv[])
{
    pthread_t t_id;
    int thread_param = 5;
    void *thr_ret;

    if(pthread_create(&t_id, NULL, thread_main, (void*)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }

    if(pthread_join(t_id, &thr_ret) != 0)
    {
        puts("pthread_join() error");
        return -1;
    }

    printf("Thread return message: %s\n", (char*)thr_ret);
    //free(thr_ret); // 전역 변수 사용시 필요없음 
    return 0;
}

void* thread_main(void *arg)
{
    int i;
    int cnt = *((int*)arg);
    //char *msg = (char*)malloc(sizeof(char) * 50); // Heap
	char msg[50] = {0};// stack: warning 발생 

    strcpy(msg, "Hello, I am thread~ \n");
    for(i=0; i < cnt; i++)
    {
        sleep(1);
        puts("running thread");
    }

    return (void*)msg;
}
