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

int main(int argc, const char * argv[]) {
    
    // 当前地址
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s \n", cwd);
    } else {
        perror("getcwd() error \n");
        return 1;
    }

    // 开始记录日志
    char *log = "FDLog 日志记录！";
    int flag = 5;
    long long localtime = 123123;
    char thread_name[] = "main";
    int thread_id = 1;
    int is_main = 1;
    FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main);
    char KEY[] = "0123456789012345";
    char IV[] = "0123456789012345";
    int success = fdlog_initialize(cwd, KEY, IV);
    if (!success) {
         printf("fd_initialize_log failture! \n");
    }
    
    fdlog(data);
//    fdlog_sync();

    return 0;
}
