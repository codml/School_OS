#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define MAX_PROCESSES 64

struct nums {
    int num1;
    int num2;
};

pthread_t tid[MAX_PROCESSES*2];

void *thread_func(void *arg) {
    long sum = ((struct nums *)arg)->num1 + ((struct nums *)arg)->num2;
    free(arg);
    pthread_exit((void *)sum);
}

int main() {
    FILE *f_temp = fopen("temp.txt", "r");
    struct timespec s_time, f_time;
    struct nums *pNum;
    int sum;
    double runtime;

    clock_gettime(CLOCK_MONOTONIC, &s_time);

    for (int i = MAX_PROCESSES; i < MAX_PROCESSES * 2; i++) {
        pNum = (struct nums *)malloc(sizeof(struct nums));
        fscanf(f_temp, "%d\n%d\n", &(pNum->num1), &(pNum->num2));
        pthread_create(tid+i, NULL, thread_func, (void *)pNum);
    }
    fclose(f_temp);

    for (long i = MAX_PROCESSES - 1; i > 0; i--) {
        pNum = (struct nums *)malloc(sizeof(struct nums));
        pthread_join(tid[i * 2], (void **)&(pNum->num1));
        pthread_join(tid[i * 2 + 1], (void **)&(pNum->num2));
        pthread_create(tid+i, NULL, thread_func, (void *)pNum);
    }
    pthread_join(tid[1], (void**)&sum);
    clock_gettime(CLOCK_MONOTONIC, &f_time);
	runtime = (f_time.tv_sec - s_time.tv_sec) + (f_time.tv_nsec - s_time.tv_nsec) / pow(10, 9);
	printf("value of thread : %d\n%lf\n", sum, runtime);
    return 0;
}