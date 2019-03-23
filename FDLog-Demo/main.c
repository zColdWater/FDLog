//
//  main.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "fd_log.h"
#include "fd_aes_helper.h"
#include "fd_directory_helper.h"


// random string
static char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

// random string wrapper
char* rand_string_alloc(size_t size)
{
    char *s = malloc(size + 1);
    if (s) {
        rand_string(s, size);
    }
    return s;
}




int main(int argc, const char * argv[]) {
    
    // 1.写入缓存文件时 如果存在内容日志时，并且开头不是 写入尾巴，需要删除上一次无效的日志
    // 2.写入日志文件时 如果存在不是写入尾巴时，也需要删除上一次无效的日志
    // 3.之前有缓存文件 再次进入 会不会 将上一次的 也一起输出。
    
    // 当前地址
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s \n", cwd);
    } else {
        perror("getcwd() error \n");
        return 1;
    }

    // 开始记录日志
    char KEY[] = "0123456789012345";
    char IV[] = "0123456789012345";
    int success = fdlog_initialize(cwd, KEY, IV);
    if (!success) {
         printf("fd_initialize_log failture! \n");
    }
    
    int i = 1;
    while (i < 9999999) {
        char *log = rand_string_alloc(99999);
        int flag = 5;
        long long localtime = 123123;
        char thread_name[] = "main";
        int thread_id = 1;
        int is_main = 1;
        FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main);
        printf("i: %d\n",i);
        fdlog(data);
        i++;
    }


    return 0;
}
