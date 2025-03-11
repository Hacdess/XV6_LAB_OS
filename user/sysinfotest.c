#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
    struct sysinfo info;

    if (sysinfo(&info) < 0)
    {
        printf("sysinfo failed\n");
        exit(1);
    }

    printf("Free memory: %ld bytes\n", info.freemem);
    printf("Number of processes: %d\n", info.nproc);
    printf("Number of open files: %d\n", info.nopenfiles);

    exit(0);
}
