#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>

#define MAX_PROCESSES 64

int main() {
    FILE *f_temp, *f_temp_w;
    int num1, num2, sum, idx;
	int pipes[MAX_PROCESSES][2];
    int exit_value;
    struct timespec s_time, f_time;
	double runtime;

	// file open
    f_temp = fopen("temp.txt", "r");
	f_temp_w = fopen("temp.txt", "a");
    if (f_temp == NULL || f_temp_w == NULL) {
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
	for (int i = 0; i < MAX_PROCESSES && fscanf(f_temp, "%d\n%d\n", &num1, &num2) != EOF; i++) {
		write(pipes[i][1], &num1, sizeof(int));
		write(pipes[i][1], &num2, sizeof(int));
		close(pipes[i][1]);
	}
	fclose(f_temp);
	clock_gettime(CLOCK_MONOTONIC, &s_time); // start time for fork()

	if (fork()) 
		wait(&exit_value); // main process: wait child's exit value
	else {
        idx = 0; // initial index
    	for (int i = 0; i < log2(MAX_PROCESSES); i++) {
            if (!fork())
                idx +=  1 << i; // update processes' unique index
        }
		
		read(pipes[idx][0], &num1, sizeof(int)); // read from pipe(first num)
		read(pipes[idx][0], &num2, sizeof(int)); // read from pipe(second num)
		close(pipes[idx][0]); // close pipe

		sum = num1 + num2; // all process doing sum
		fprintf(f_temp_w, "%d\n", sum);

		while (wait(&exit_value) >= 0) { // until process has no child
			sum += exit_value >> 8; // my sum + child's sum
			fprintf(f_temp_w, "%d\n", sum);
		}
		exit(sum); // exit process and pass value
	}
	fclose(f_temp_w); // close file
	clock_gettime(CLOCK_MONOTONIC, &f_time); // finish time
	runtime = (f_time.tv_sec - s_time.tv_sec) + (f_time.tv_nsec - s_time.tv_nsec) / pow(10, 9);
	printf("value of fork : %d\n%lf\n", exit_value >> 8, runtime); // return result
    return 0;
}
