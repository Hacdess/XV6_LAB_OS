#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"
#include "kernel/stat.h"

int main(void) {
    struct sysinfo info;

    if (sysinfo(&info) < 0) {
        printf("sysinfo failed\n");
        exit(1);
    }

    printf("Free memory: %ld bytes\n", info.freemem);
    printf("Number of processes: %ld\n", info.nproc);
    printf("Number of open files: %ld\n", info.nopenfiles);

    exit(0);
}