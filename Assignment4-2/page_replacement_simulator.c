#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000

// queue for pages //
// actually, it's simillar with linked list //
// there is no empty place arr[0, tail] //
typedef struct {
	int		arr[MAX_SIZE];
	int		tail;
	int		len;
} queue;

// initialize queue //
void init(queue *q, int size) {
	q->tail = 0;
	q->len = size;
}

// push to idx //
// if idx < tail, push back arr[idx, tail] //
// and push //
void push(queue *q, int num, int idx) {
	for (int i = q->tail; i > idx; i--)
		q->arr[i] = q->arr[i-1];
	q->arr[idx] = num;
	q->tail++;
}

// pop from idx //
// if idx < tail, move forward arr[idx, tail] //  
void pop(queue *q, int idx) {
	for (int i = idx; i < q->tail; i++)
		q->arr[i] = q->arr[i+1];
	q->tail--;
}

// determine full or not //
int isFull(queue *q) {
	if (q->tail == q->len)
		return 1;
	else
		return 0;
}

/// return idx of queue, or -1 ///
int isHere(queue *q, int num) {
	for (int i = 0; i < q->tail; i++) {
		if (num == q->arr[i])
			return i;
	}
	return -1;
}

// optimal algorithm: evict farest frame //
void OPT(int max, int len, int string[]) {
	int page_fault = 0;
	int idx, delete_idx;
	char *visit;
	queue q;

	printf("Optimal Algorithm: \n");
	init(&q, max);

	for (int i = 0; i < len; i++) {
		// if page frame is not in queue //
		if (isHere(&q, string[i]) == -1) {
			page_fault++;
			// if queue is full: need to evict //
			if (isFull(&q)) {
				// visit: record queue's visit //
				visit = (char *)malloc(q.len);
				memset(visit, 0, q.len);

				// check future page using //
				for (int j = i+1; j < len; j++) {
					if ((idx = isHere(&q, string[j])) != -1 && visit[idx] == 0) {
						visit[idx] = 1;
						delete_idx = idx;
					}
				}

				// if there is a page which will never visit in future, evict //
				for (idx = 0; idx < q.len; idx++) {
					if (visit[idx] == 0) {
						pop(&q, idx);
						break;
					}
				}
				free(visit);

				// evict page used in longest period //
				if (idx == q.len)
					pop(&q, delete_idx);
			}
			// push queue new page //
			push(&q, string[i], q.tail);
		}
	}

	// print page fault //
	printf("Number of Page Faults: %d\n", page_fault);
	printf("Page Fault Rate: %.2lf%%\n", page_fault * 100 / (double)len);
}

void FIFO(int max, int len, int string[]) {
	int page_fault = 0;
	queue q;

	printf("FIFO Algorithm: \n");
	init(&q, max);

	for (int i = 0; i < len; i++) {
		// if page is not in queue // 
		if (isHere(&q, string[i]) == -1) {
			page_fault++;
			// if full//
			if (isFull(&q))
				pop(&q, 0); // evict oldest page
			push(&q, string[i], q.tail); // push new page
		}
	}

	printf("Number of Page Faults: %d\n", page_fault);
	printf("Page Fault Rate: %.2lf%%\n", page_fault * 100 / (double)len);
}

void LRU(int max, int len, int string[]) {
	int page_fault = 0;
	int idx;
	queue q;

	printf("LRU Algorithm: \n");
	init(&q, max);

	for (int i = 0; i < len; i++) {
		// if page is not in queue //
		if ((idx = isHere(&q, string[i])) == -1) {
			page_fault++;
			// same with FIFO //
			if (isFull(&q))
				pop(&q, 0);
			push(&q, string[i], q.tail);
		} else {
			// if page is in queue //
			pop(&q, idx);
			// push page to highest place in queue //
			push(&q, string[i], q.tail);
		}
	}

	printf("Number of Page Faults: %d\n", page_fault);
	printf("Page Fault Rate: %.2lf%%\n", page_fault * 100 / (double)len);
}

void CLOCK(int max, int len, int string[]) {
	int page_fault = 0;
	int idx, evict;
	queue q;
	queue bit_queue; // store page's use bit
	
	printf("Clock Algorithm: \n");
	init(&q, max);
	init(&bit_queue, max);
	evict = 0; // point evict position

	for (int i = 0; i < len; i++) {
		// if page is not in queue //
		if ((idx = isHere(&q, string[i])) == -1) {
			page_fault++;
			// if full: evict
			if (isFull(&q)) {
				// traversal queue: evict -> tail -> evict-1 //
				// like circular queue //
				// if use bit: 0->evict //
				// else(use bit: 1) use bit -> 0 //
				for (idx = evict; idx < q.len; idx++) {
					if (bit_queue.arr[idx] == 0) {
						pop(&q, idx);
						pop(&bit_queue, idx);
						break;
					} else
						bit_queue.arr[idx] = 0;
				}
				if (idx == q.len) {
					for (idx = 0; idx < evict; idx++) {
						if (bit_queue.arr[idx] == 0) {
							pop(&q, idx);
							pop(&bit_queue, idx);
							break;
						} else
							bit_queue.arr[idx] = 0;
					}
					if (idx == evict) {
						pop(&q, idx);
						pop(&bit_queue, idx);
					}
				}
				////////////////////
			}
			if (idx == -1) {
				// just push to queue, not full->evict->push //
				push(&q, string[i], q.tail);
				push(&bit_queue, 0, bit_queue.tail);
			} else {
				// idx: evict place -> also push place for new page //
				push(&q, string[i], idx);
				push(&bit_queue, 0, idx);
			}
			// update evict pointer: +1 or go to head //
			evict = idx;
			if (evict == q.tail - 1)
				evict = 0;
			else
				evict++;
		} else
			bit_queue.arr[idx] = 1; // reference: use bit->1
	}

	printf("Number of Page Faults: %d\n", page_fault);
	printf("Page Fault Rate: %.2lf%%\n", page_fault * 100 / (double)len);
}

int main(int argc, char **argv) {
	FILE	*fp;
	int		max;
	int		len; // string length
	char	buf[MAX_SIZE*5];
	char	*ptr;
	int		string[MAX_SIZE];

	/// argument number error ///
    if (argc != 2) {
        perror("only input file needed: file name\n");
        exit(0);
    }
	// file open //
	if (!(fp = fopen(argv[1], "r"))) {
        perror("file open failed.\n");
        exit(0);
    }
	// file read and fill size(max), string //
	fscanf(fp, "%d\n", &max);
	fgets(buf, sizeof(buf), fp);
	len = 0;
	ptr = strtok(buf, " \n");
	while (ptr != NULL) {
		string[len++] = atoi(ptr);
		ptr = strtok(NULL, " \n");
	}
	fclose(fp);

	/// algorithms ///
	OPT(max, len, string);
	printf("\n");
	FIFO(max, len, string);
	printf("\n");
	LRU(max, len, string);
	printf("\n");
	CLOCK(max, len, string);

	return 0;
}