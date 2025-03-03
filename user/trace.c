#include "user.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(2, "Usage: trace <mask> <program> [args...]\n");
        exit(1);
    }

    int mask = atoi(argv[1]);  // Lấy mask từ argv[1]
    trace(mask);  // Gọi trace system call

    // Chuẩn bị chạy chương trình được chỉ định
    exec(argv[2], &argv[2]);
    fprintf(2, "exec %s failed\n", argv[2]);
    exit(1);
}
