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
#include "fd_rsa_helper.h"
#include <mbedtls/pk.h>


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
    
    // 对外是小端
    // 释放堆内存

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
    int success = fdlog_initialize_by_rsa(cwd, KEY, IV, 1);
    if (!success) {
        printf("fd_initialize_log failture! \n");
        return 0;
    }

    // 是否开启Debug模式
    fdlog_open_debug(0);
    // 保存最近几天的日志
    fdlog_save_recent_days(7);
    // 日志最大尺寸
    fdlog_set_logfile_max_size(1024*500); // 1MB

    // 存储日志的文件夹路径
    char *temp = (char *)calloc(1, 1024);
    fdlog_log_folder_path(temp);
    printf("temp: %s \n",temp);
    free(temp);
    temp = NULL;

    // 写入日志
    int i = 1;
//    while (i < 19999) {
//        char *log = rand_string_alloc(30);
//        int flag = 5;
//        long long localtime = 123123;
//        char thread_name[] = "main";
//        int thread_id = 1;
//        int is_main = 1;
//        int level = 0;
//        FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main, level);
//        printf("i: %d\n",i);
//        fdlog(data);
//        i++;
//    }

    unsigned char ctr[] = "UFkqJqoVaDRevQ42j7jFd1adKwwjw079rEulQ03ZTT4RyN6u1hmvUpI2MKy6RE5hBnAc5Ra+5Ks8bDKzuxudOO/6RpTJyedB/fNtN+UaQVmgarNkSek+8aO4irX1DMIgEqU8OqbzPEIBjrlZaT68wKqVLG32ogACuoyAGjVS+f0=";
    unsigned char result[MBEDTLS_MPI_MAX_SIZE];
    memset(result, 0, MBEDTLS_MPI_MAX_SIZE);
    
    int decodeRet = fd_rsa_decode(ctr,result,MBEDTLS_MPI_MAX_SIZE);
    printf("decodeRet: %d \n",decodeRet);
    printf("result: %s \n",result);


    return 0;
}

