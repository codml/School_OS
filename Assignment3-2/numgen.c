#include <stdio.h>

#define MAX_PROCESSES 8

int main() {
    FILE *f_write = fopen("temp.txt", "w"); // file open

    for (int i = 0; i < MAX_PROCESSES*2; i++)
        fprintf(f_write, "%d\n", i+1); // write 1~MAX_PROCESSES*2 to temp.txt

    fclose(f_write); // close file stream
    return 0;
}
