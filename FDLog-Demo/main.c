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
#include "fd_pk.h"
#include "fd_rsa.h"
#include "fd_entropy.h"
#include "fd_ctr_drbg.h"
#include "fd_base64.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// 解密日志文件流程 ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 读文件
//    char* content = (char*)calloc(1, 128);
//    FILE *file_temp = fopen("/Users/yongpeng.zhu/Library/Developer/Xcode/DerivedData/FDLog-Demo-gyroeflfuxxqyicqdebrvreigkbk/Build/Products/Debug/fdlog_v1/logs/20190509/201905091", "ab+");
//    if (NULL != file_temp) {  //初始化文件流开启
//        fseek(file_temp, 9, SEEK_SET);
//        fread(content, 128, sizeof(char), file_temp);
//        fclose(file_temp);
//    }
//
//    unsigned char results[MBEDTLS_MPI_MAX_SIZE];
//    memset(results, 0, MBEDTLS_MPI_MAX_SIZE);
//    // base64 encode 后字节数
//    size_t olen1 = 0;
//    int ret1 = mbedtls_base64_encode((unsigned char*)results, sizeof(results),&olen1,content, 128);
//    printf("results: %s \n",results);
//    printf("\n");


// 做验证的步骤:
// 1.截取文件一个单元的16进制字节流。
// 2.对字节流做Base64，然后去云端decode，对比流是否正确。 https://cryptii.com/pipes/base64-to-hex
// 3.用这个Base64字符串去做AES128解密，得到压缩文件流的Base64字符串。 https://www.devglan.com/online-tools/aes-encryption-decryption
// 4.在用这个压缩文件流的Base64字符串去做云端做解压得到密文实体。 http://www.txtwizard.net/compression

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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

    unsigned char ctr[] = "Lnwak+kfNWuUcqiRJYIfvDSbsACL4hPdnwaTQw5EX6VhLNYJL63vaLbK5oop7fqWDd7IM1LmHFMrC0lFkjv3C4jirtizPtB3hECDLBb6bCdyyiecXuT7c5vJGiJVBc73BgfX2u7OOne93INxGe/UKQhj5SXv5XaEdhctlIwBh4I=";
    int success = fdlog_initialize_by_rsa(cwd, ctr);
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
    while (i < 20000) {
//        char *log = rand_string_alloc(30);
//        int flag = 5;
//        long long localtime = 123123;
//        char thread_name[] = "main";
//        int thread_id = 1;
//        int is_main = 1;
//        int level = 0;
        FD_Construct_Data *data = fd_construct_json_data("1", "1", 1, "1", 1, 1, 1, 1, "1", "1", "1", "1", "1", "1", "1", "1", "1", "1");
        fdlog(data);
        i++;
    }
    
    return 0;
}

