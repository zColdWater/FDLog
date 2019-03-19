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



/**
 读取缓存头文件信息
 文件内容长度 & 头信息内容
 
 @param mmap_buffer mmap 绑定指针 | 指向缓存文件开头
 @return 1成功 0失败
 */
int adjust_mmap_tailer(unsigned char *mmap_buffer) {
    
    unsigned char *temp = mmap_buffer;
    if (*temp == FD_MMAP_FILE_HEADER) {
        temp += 1;
        /// 读取头部信息长度
        mmap_header_content_len_ptr = (int*)temp;
        temp += sizeof(int);
        
        /// 读取头部内容
        memcpy(mmap_header_content_ptr, temp, *mmap_header_content_len_ptr);
        
        temp += *mmap_header_content_len_ptr;
        if (*temp == FD_MMAP_FILE_TAILER) {
            printf("READ FD_MMAP_FILE_TAILER ✅\n");
            temp += 1;
            
            // 判断是否是 文件内容的数据的头部
            if (*temp == FD_MMAP_FILE_CONTENT_HEADER) {
                temp += 1;
                printf("READ FD_MMAP_FILE_CONTENT_HEADER ✅\n");
                mmap_content_len_ptr = (int*)temp;
                printf("mmap_content_len_ptr:%d\n",*mmap_content_len_ptr);
                temp += sizeof(int);
                
                if (*temp == FD_MMAP_FILE_CONTENT_TAILER) {
                    printf("READ FD_MMAP_FILE_CONTENT_TAILER ✅\n");
                    temp += 1;
                    
                    mmap_tailer_ptr = mmap_ptr;
                    int mmap_header_len = 2 + sizeof(int) + *mmap_header_content_len_ptr;
                    int mmap_content_len = 2 + sizeof(int) + *mmap_content_len_ptr;
                    
                    // 移动距离
                    int move_len = mmap_header_len + mmap_content_len;
                    printf("move_len:%d \n",move_len);
                    mmap_tailer_ptr += move_len;
                    return 1;
                }
            }
        }
    }
    return 0;
}


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

