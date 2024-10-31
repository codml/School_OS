#include <stdio.h>

#define MAX_PROCESSES 8

int main() {
    FILE *f_write = fopen("temp.txt", "wt");

    for (int i = 0; i < MAX_PROCESSES*2; i++) {
        if ((i+1) / 10 == 0)
            fprintf(f_write, "  ");
        else if (((i+1) / 10) / 10 == 0)
            fprintf(f_write, " ");
        fprintf(f_write, "%d\n", i+1);
    }

    fclose(f_write);
    return 0;
}