#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_SIZE 1000

struct process {
	int pid;
	int arrival;
	int burst;
	int remain;
	int start;
	int finish;
	int waiting;
	int turnaround;
	int response;
	int end;
};

struct process * init_list(FILE *f_temp, int *len) {
	char 			buf[30];
	struct process	p;
	struct process	*ptr;
	int 			cnt = 0;

	p.end = 0;
	ptr = (struct process *)malloc(sizeof(struct process));
	while (fscanf(f_temp, "%d %d %d\n", &(p.pid), &(p.arrival), &(p.burst)) != EOF) {
		p.remain = p.burst;
		ptr = (struct process *)realloc(ptr, sizeof(struct process)*(++cnt));
		ptr[cnt-1] = p;
	}
	*len = cnt;
	return ptr;
}

int FCFS(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;

	printf("Gantt Chart:\n|");

	for (; fin_cnt < len; time++) {
		if (running == -1 || list[running].remain == 0) {
			if (running != -1 && list[running].remain == 0) {
				list[running].finish = time;
				list[running].end = 1;
				running = -1;
				fin_cnt++;
			}
			for (int i = 0; i < len; i++) {
				if (list[i].arrival <= time && list[i].end == 0) {
					list[i].start = time;
					running = i;
					break;
				}
			}
			if (running == -1) {
				if (fin_cnt < len)
					printf(" |");
				continue;
			}
		}
		list[running].remain -= 1;
		printf(" P%d |", list[running].pid);
	}
	
	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return len;
}

int SJF(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;

	printf("Gantt Chart:\n|");

	for (; fin_cnt < len; time++) {
		if (running == -1 || list[running].remain == 0) {
			if (running != -1 && list[running].remain == 0) {
				list[running].finish = time;
				list[running].end = 1;
				running = -1;
				fin_cnt++;
			}
			int min = INT_MAX;
			int min_idx = -1;
			for (int i = 0; i < len; i++) {
				if (list[i].arrival <= time && list[i].end == 0 && min > list[i].burst) {
					min = list[i].burst;
					min_idx = i;
				}
			}
			running = min_idx;
			if (running == -1) {
				if (fin_cnt < len)
					printf(" |");
				continue;
			}
			list[running].start = time;
		}
		list[running].remain -= 1;
		printf(" P%d |", list[running].pid);
	}
	
	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].finish - list[i].arrival - list[i].burst;
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return len;
}

int SRTF(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;
	int context_switch = 0;

	printf("Gantt Chart:\n|");

	for (; fin_cnt < len; time++) {
		if (running != -1 && list[running].remain == 0) {
			list[running].finish = time;
			list[running].end = 1;
			running = -1;
			context_switch++;
			fin_cnt++;
		}
		int min = INT_MAX;
		int min_idx = -1;
		for (int i = 0; i < len; i++) {
			if (list[i].arrival <= time && list[i].end == 0 && min > list[i].remain) {
				min = list[i].remain;
				min_idx = i;
			}
		}
		if (min_idx != -1) {
			if (running != -1 && running != min_idx)
				context_switch++;
			running = min_idx;
			if (list[running].burst == list[running].remain)
				list[running].start = time;
		}
		if (running == -1) {
			if (fin_cnt < len)
				printf(" |");
			continue;
		}
		list[running].remain -= 1;
		printf(" P%d |", list[running].pid);
	}
	
	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].finish - list[i].arrival - list[i].burst;
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return context_switch;
}

int RR(struct process *list, int len, int time_slice) {
	int context_switch = 0;

	return context_switch;
}

int main(int argc, char **argv) {
    FILE			*f_temp;
	struct process	*list;
	int 			len;
	int				context_switch;

    if (argc < 3) {
        perror("2 or 3 arguments are needed: file name / CPU algorithm / (if RR, time slice).\n");
        exit(0);
    }

    if (!(f_temp = fopen(argv[1], "r"))) {
        perror("file open failed.\n");
        exit(0);
    }
	list = init_list(f_temp, &len);
	fclose(f_temp);

	if (!strcmp(argv[2], "FCFS"))
		context_switch = FCFS(list, len);
	else if (!strcmp(argv[2], "SJF"))
		context_switch = SJF(list, len);
	else if (!strcmp(argv[2], "SRTF"))
		context_switch = SRTF(list, len);
	else if (!strcmp(argv[2], "RR")) {
		if (argc == 4)
			context_switch = RR(list, len, atoi(argv[3]));
		else {
			perror("RR needs time quantum\n");
			exit(0);
		}
	}
	else {
		perror("Wrong algorithm name.\n");
		exit(0);
	}

	double turnaround = 0;
	double waiting = 0;
	double response = 0;
	double burst = 0;
	double total;
	int max_finish = 0;

	for (int i = 0; i < len; i++)
		turnaround += list[i].turnaround;
	for (int i = 0; i < len; i++)
		waiting += list[i].waiting;
	for (int i = 0; i < len; i++)
		response += list[i].response;
	for (int i = 0; i < len; i++)
		burst += list[i].burst;
	for (int i = 0; i < len; i++)
		max_finish = max_finish > list[i].finish ? max_finish : list[i].finish;
	total = max_finish + 0.1*context_switch;

	printf("Average Waiting Time = %.2lf\n", waiting / (double)len);
	printf("Average Turnaround Time = %.2lf\n", turnaround / (double)len);
	printf("Average Response Time = %.2lf\n", response / (double)len);
	printf("CPU Utilization = %.2lf%%\n", (double)100 * burst / total);	
	free(list);

	return 0;
}