#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>

#define MAX_PROCESSES 4

int get_idx(int idx, int i) {
    int po;

	po = 1;
    while (i--)
        po *= 2;
    return idx + po;
}

int main() {
    FILE *f_temp;
    char buf[12];
    int num1, num2, sum, idx;
	int pipes[MAX_PROCESSES][2];
    int exit_value;
    struct timespec s_time, f_time;

	clock_gettime(CLOCK_MONOTONIC, &s_time);

	// 파일 오픈
    f_temp = fopen("temp.txt", "r");
    if (f_temp == NULL) {
        perror("Failed to open file");
        return 1;
    }

    // generate pipes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            return 1;
        }
    }

	// root process: read from file and write to pipe
	for (int i = 0; i < MAX_PROCESSES && fscanf(f_temp, "%d\n%d\n", &num1, &num2) != EOF; i++) { // 두 수 읽기
		write(pipes[i][1], &num1, sizeof(int));
		write(pipes[i][1], &num2, sizeof(int));
		close(pipes[i][1]);
	}

	fclose(f_temp);

	if (fork()) 
		wait(&exit_value);
	else {
        idx = 0;
    	for (int i = 0; i < log2(MAX_PROCESSES); i++) {
            if (!fork())
                idx = get_idx(idx, i);
        }
		
		read(pipes[idx][0], &num1, sizeof(int));
		read(pipes[idx][0], &num2, sizeof(int));
		close(pipes[idx][0]);

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