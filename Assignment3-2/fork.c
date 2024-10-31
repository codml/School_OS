#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_PROCESSES 8

int pow_i(int n, int i) {
    int ret = 1;
    while (i--)
        ret *= n;
    return ret;
}

int main() {
    FILE *f_temp;
    char buf[12];
    int num1, num2, sum, f_idx;
    int exit_value;
    struct timespec s_time, f_time;

	clock_gettime(CLOCK_MONOTONIC, &s_time);

	if (fork()) 
		wait(&exit_value);
	else {
        f_idx = 0;
    	for (int i = 0; i < log2(MAX_PROCESSES); i++) {
            if (!fork())
                f_idx += pow_i(2, i);
        }
				
        f_temp = fopen("temp.txt", "r");
        fseek(f_temp, f_idx * 8, SEEK_SET);
		fread(buf, 4, 1, f_temp);
		num1 = atoi(buf);
        fread(buf, 4, 1, f_temp);
        num2 = atoi(buf);
        fclose(f_temp);

		sum = num1 + num2;
		while (wait(&exit_value) >= 0)
			sum += exit_value >> 8;
		exit(sum);
	}
	clock_gettime(CLOCK_MONOTONIC, &f_time);
	double runtime = (f_time.tv_sec - s_time.tv_sec) + (f_time.tv_nsec - s_time.tv_nsec) / pow(10, 9);
	printf("sum: %d, second: %lf\n", exit_value >> 8, runtime);
    return 0;
}