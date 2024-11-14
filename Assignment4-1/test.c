#include <unistd.h>
#include <sys/types.h>
#include <linux/unistd.h>

int main () {
    syscall(336, getpid());
    return 0;
}