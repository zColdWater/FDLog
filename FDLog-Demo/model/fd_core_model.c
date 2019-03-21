//
//  fd_core_model.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/3/8.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_core_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// FDLog: 日志核心Model
FDLOGMODEL *model = NULL;

/// FDLog: 日志文件夹地址
char *log_folder_path = NULL;

/// FDLog: 日志文件字节长度
long *log_file_len = NULL;

/// FDLog: 日志文件地址
char *log_file_path = NULL;

/// FDLog: 缓存文件路径
char *mmap_cache_file_path = NULL;

/// FDLog: MMAP文件地址指向 位置不改变永远指向缓存文件头
unsigned char *mmap_ptr = NULL;

/// FDLog: MMAP头部内容 指针 位置不改变永远指向缓存文件 内容最后位置
unsigned char *mmap_tailer_ptr = NULL;

/// FDLog: MMAP内容长度 指针 位置不改变永远指向缓存文件 内容长度
int *mmap_content_len_ptr = NULL;

/// FDLog: MMAP 当前正在写入的那条日志的长度
int *mmap_current_log_len_ptr = NULL;

/// FDLog: MMAP头部内容长度 指针 位置不改变永远指向缓存文件 头部长度
int *mmap_header_content_len_ptr = NULL;

/// FDLog: MMAP头部内容 指针 位置不改变永远指向缓存文件 头部内容
char *mmap_header_content_ptr = NULL;




int update_mmap_content_len(int increase_content_len) {
    if ((mmap_ptr != NULL) && (mmap_content_len_ptr != NULL) && (mmap_header_content_ptr != NULL) && (mmap_header_content_len_ptr != NULL) && (mmap_tailer_ptr != NULL)) {
        /// 更新内容长度
        int new_len = *mmap_content_len_ptr + increase_content_len;
        memcpy(mmap_content_len_ptr,&(new_len),sizeof(int));
        printf("mmap_content_len_ptr:%d \n",*mmap_content_len_ptr);
        
        return 1;
    }
    return 0;
}

