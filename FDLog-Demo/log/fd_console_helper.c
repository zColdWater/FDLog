#include "fd_console_helper.h"

static int fd_is_debug = 0;

int fd_printf(char *fmt, ...) {
    int cnt = 0;
    if (fd_is_debug) {
        va_list argptr;
        va_start(argptr, fmt);
        cnt = vprintf(fmt, argptr);
        va_end(argptr);
    }
    return (cnt);
}

void fd_set_debug(int debug) {
    fd_is_debug = debug;
}
