#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_SIZE 1000

/// store informations for process ///
struct process {
	/// read from file ///
	int pid;
	int arrival;
	int burst;
	///
	/// acquire in running time ///
	int remain;
	int start;
	int finish;
	///
	/// when simulation finish, acquired by calculation ///
	int waiting;
	int turnaround;
	int response;
	///
	/// indicate the process finish or not-> init by 0 ///
	int end;
	///
};

/// read file and make process array ///
struct process * init_list(FILE *f_temp, int *len) {
	struct process	p;
	struct process	*ptr;
	int 			cnt = 0;

	p.end = 0;
	/// init size = 1 ///
	ptr = (struct process *)malloc(sizeof(struct process));
	/// read file ///
	while (fscanf(f_temp, "%d %d %d\n", &(p.pid), &(p.arrival), &(p.burst)) != EOF) {
		p.remain = p.burst; // first remain time
		/// increase size ///
		ptr = (struct process *)realloc(ptr, sizeof(struct process)*(++cnt));
		ptr[cnt-1] = p;
	}
	*len = cnt; // write size of processes in 'len'
	return ptr; // return process list pointer
}

int FCFS(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;

	printf("Gantt Chart:\n|");
	
	/// insertion sort by arrival time ///
	for (int i = 1; i < len; i++) {
		struct process pivot = list[i];
		int j;
		for (j = i - 1; j >= 0; j--) {
			if (list[j].arrival > pivot.arrival)
				list[j+1] = list[j];
			else
				break;
		}
		list[j+1] = pivot;
	}

	/// 1 iteration: 1 milisecond in simulation ///
	for (; fin_cnt < len; time++) {
		if (running == -1 || list[running].remain == 0)
		{ // no running process(case 1) or running process stopped(case 2)
			if (running != -1 && list[running].remain == 0) // case 2
			{
				list[running].finish = time;
				list[running].end = 1;
				running = -1;
				fin_cnt++; // finished process count
			}
			for (int i = 0; i < len; i++)
			{ // find process to start //
				if (list[i].arrival <= time && list[i].end == 0) {
					list[i].start = time;
					running = i;
					break;
				}
			}
			if (running == -1) {
				// no runnable process //
				if (fin_cnt < len)
					printf(" |");
				continue;
			}
		}
		list[running].remain -= 1; // running
		printf(" P%d |", list[running].pid);
	}
	
	/// calculate each processes' waiting, response, turnaround time ///
	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return len; // In FCFS, the number of process == context switch
}

int SJF(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;

	printf("Gantt Chart:\n|");

	/// same with FCFS, 1 iteration : 1 milisecond /// 
	for (; fin_cnt < len; time++)
	{	// also same with FCFS, find runnable process //
		if (running == -1 || list[running].remain == 0) {
			if (running != -1 && list[running].remain == 0) {
				list[running].finish = time;
				list[running].end = 1;
				running = -1;
				fin_cnt++;
			}
			/// runnable process: from current time, arrived before ///
			/// SJF select SHORTEST process in runnable ///
			/// min: min burst time, min_idx: index in process list ///
			int min = INT_MAX;
			int min_idx = -1;
			for (int i = 0; i < len; i++) {
				if (list[i].arrival <= time && list[i].end == 0 && min > list[i].burst) {
					min = list[i].burst;
					min_idx = i;
				}
			}
			running = min_idx;
			if (running == -1) { // no runnable
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
		list[i].waiting = list[i].start - list[i].arrival; // same with FCFS: non-preemptive alghrithm
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return len; // same with FCFS: non-preemptive alghrithm
}

int SRTF(struct process *list, int len) {
	int fin_cnt = 0;
	int time = 0;
	int running = -1;
	int context_switch = 0;

	printf("Gantt Chart:\n|");

	for (; fin_cnt < len; time++) {
		/// running process exited ///
		if (running != -1 && list[running].remain == 0) {
			list[running].finish = time;
			list[running].end = 1;
			running = -1;
			fin_cnt++;
		}

		/// find SHORTEST(unlike SJF, remain time, not burst time) process ///
		int min = INT_MAX;
		int min_idx = -1;
		for (int i = 0; i < len; i++) {
			if (list[i].arrival <= time && list[i].end == 0 && min > list[i].remain) {
				min = list[i].remain;
				min_idx = i;
			}
		}

		/// find SHORTEST process ///
		if (min_idx != -1) {
			if (running != min_idx) // context switch orcurred
				context_switch++;
			running = min_idx;
			if (list[running].burst == list[running].remain) // first run: record start time
				list[running].start = time;
		}
		else {
			if (fin_cnt < len)
				printf(" |");
			continue;
		}
		list[running].remain -= 1;
		printf(" P%d |", list[running].pid);
	}
	
	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].finish - list[i].arrival - list[i].burst; // preemptive
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return context_switch; // preemptive
}

int RR(struct process *list, int len, int time_slice) {
	int context_switch = 0;
	int fin_cnt = 0;
	int running = -1;
	int time = 0;
	int prev_run; // to recognize context switch
	int cur_slice; // time slice remaining for running process
	/// queue ///
	int queue[MAX_SIZE];
	int head = 0;
	int tail = 0;
	///

	printf("Gantt Chart:\n|");

	for (; fin_cnt < len; time++) {
		// push processes arrived in queue
		for (int i = 0; i < len; i++) {
			if (time == list[i].arrival)
				queue[tail++] = i;
		}

		// running job exited //
		if (running != -1 && list[running].remain == 0) {
			list[running].finish = time;
			list[running].end = 1;
			running = -1;
			fin_cnt++;
		} // time quantum done for running process(not finish) //
		else if(running != -1 && cur_slice == 0) {
			// push time slice expired job
			queue[tail++] = running;
			running = -1;
		}

		if (running == -1) {
			cur_slice = time_slice; // remaining time slice init 
			/// queue is empty: no runnable process ///
			if (head == tail) {
				if (fin_cnt < len)
					printf(" |");
				continue;
			}
			/// pop from queue: select process to run ///
			running = queue[head++];
			if (list[running].burst == list[running].remain)
				list[running].start = time;
			if (prev_run != running) // context switch(ex: prev_run(P1) != running(P2))
				context_switch++;
		}
		list[running].remain -= 1;
		prev_run = running; // record prev_run
		cur_slice--;
		printf(" P%d |", list[running].pid);
	}

	for (int i = 0 ; i < len; i++)
		list[i].waiting = list[i].finish - list[i].arrival - list[i].burst; // preemptive
	for (int i = 0; i < len; i++)
		list[i].response = list[i].start - list[i].arrival;
	for (int i = 0; i < len; i++)
		list[i].turnaround = list[i].finish - list[i].arrival;

	printf("\n");
	return context_switch;
}

int main(int argc, char **argv) {
    FILE			*f_temp;
	struct process	*list;
	int 			len;
	int				context_switch;

	/// argument number error ///
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
		else { // no time slice parameter for RR; error
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
	/// total burst time: Total CPU Busy time ///
	for (int i = 0; i < len; i++)
		burst += list[i].burst;
	/// total time: max(finish time) + context switch overhead ///
	for (int i = 0; i < len; i++)
		max_finish = max_finish > list[i].finish ? max_finish : list[i].finish;
	total = max_finish + 0.1*context_switch;
	///
	
	printf("Average Waiting Time = %.2lf\n", waiting / (double)len);
	printf("Average Turnaround Time = %.2lf\n", turnaround / (double)len);
	printf("Average Response Time = %.2lf\n", response / (double)len);
	printf("CPU Utilization = %.2lf%%\n", (double)100 * burst / total);	
	free(list);

	return 0;
}
