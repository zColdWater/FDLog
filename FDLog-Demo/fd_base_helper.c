#include "fd_base_helper.h"
#include "fd_console_helper.h"
#include <sys/time.h>
#include <memory.h>

#define FD_BYTEORDER_NONE  0
#define FD_BYTEORDER_HIGH 1
#define FD_BYTEORDER_LOW 2

void ReverseArray(char arr[], long long size)
{
    for (int i = 0; i < size/2; ++i)
    {
        char temp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = temp;
    }
}

//获取毫秒时间戳
long long fd_get_system_current(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long time = ((long long) tv.tv_sec) * 1000 + ((long long) tv.tv_usec) / 1000;
    return time;
}

//是否为空字符串
int fd_is_string_empty(char *item) {
    int flag = 1;
    if (NULL != item && strnlen(item, 10) > 0) {
        flag = 0;
    }
    return flag;
}

// 检查CPU的字节序
// 计算机硬件有两种储存数据的方式：大端字节序（big endian）和小端字节序（little endian）。
// 举例来说，数值0x2211使用两个字节储存：高位字节是0x22，低位字节是0x11。
int fd_cpu_byteorder(void) {
    static int FD_BYTEORDER = FD_BYTEORDER_NONE;
    if (FD_BYTEORDER == FD_BYTEORDER_NONE) {
        union {
            int i;
            char c;
        } t;
        t.i = 1;
        if (t.c == 1) {
            FD_BYTEORDER = FD_BYTEORDER_LOW;
            fd_printf("cpu_byteorder_clogan > system is a low byteorder\n");
        } else {
            FD_BYTEORDER = FD_BYTEORDER_HIGH;
            fd_printf("cpu_byteorder_clogan > system is a high byteorder\n");
        }
    }
    return FD_BYTEORDER;
}

/**
 * 调整字节序,默认传入的字节序为低字节序,如果系统为高字节序,转化为高字节序
 * c语言字节序的写入是低字节序的,读取默认也是低字节序的
 * java语言字节序默认是高字节序的
 * @param data data
 */
void fd_adjust_byteorder(char data[4]) {
    if (fd_cpu_byteorder() == FD_BYTEORDER_HIGH) {
        char *temp = data;
        char data_temp = *temp;
        *temp = *(temp + 3);
        *(temp + 3) = data_temp;
        data_temp = *(temp + 1);
        *(temp + 1) = *(temp + 2);
        *(temp + 2) = data_temp;
    }
}
