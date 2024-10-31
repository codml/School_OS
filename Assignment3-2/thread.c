#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define MAX_PROCESSES 8

struct nums {
    int num1;
    int num2;
};

pthread_t tid[MAX_PROCESSES*2];

void *thread_read_func(void *arg) {
    int sum = ((struct nums *)arg)->num1 + ((struct nums *)arg)->num2;
    pthread_exit((void *)sum);
}

void *thread_join_func(void *arg) {
    int num1, num2, sum;

    pthread_join(tid[(int)arg * 2], &num1);
    pthread_join(tid[(int)arg * 2 + 1], &num2);
    pthread_detach(tid[(int)arg * 2]);
    pthread_detach(tid[(int)arg * 2 + 1]);
    sum = num1 + num2;
    pthread_exit((void *)sum);
}

int main() {
    FILE *f_temp = fopen("temp.txt", "r");
    char buf[15];
    int sum;

    for (int i = MAX_PROCESSES; i < MAX_PROCESSES * 2; i++) {
        struct nums s_num;
        fgets(buf, sizeof(buf), f_temp);
        s_num.num1 = atoi(buf);
        fgets(buf, sizeof(buf), f_temp);
        s_num.num2 = atoi(buf);
        pthread_create(tid+i, NULL, thread_read_func, (void *)&s_num);
    }
    fclose(f_temp);

    for (int i = MAX_PROCESSES - 1; i > 0; i--)
        pthread_create(tid+i, NULL, thread_join_func, (void *)i);
    pthread_join(tid[1], &sum);
}