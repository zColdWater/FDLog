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

// 日志核心Model
FDLOGMODEL *model = NULL;

/// FDLog: 日志文件夹地址
char *log_folder_path = NULL;

/// FDLog: 日志文件字节长度
long *log_file_len = NULL;

/// FDLog: 日志文件地址
char *log_file_path = NULL;

/// FDLog: 缓存文件路径
char *mmap_cache_file_path = NULL;

/// MMAP文件地址指向
/// 位置不改变永远指向缓存文件头
unsigned char *mmap_ptr = NULL;

/// MMAP头部内容 指针
/// 位置不改变永远指向缓存文件 内容最后位置
unsigned char *mmap_tailer_ptr = NULL;

/// MMAP剩余数据长度 指针
/// 位置不改变 指向剩余数据长度
unsigned char *mmap_remain_ptr = NULL;

/// MMAP从文件头到最后一条日志记录长度的位置 指针
unsigned char *mmap_last_log_content_len_distance_ptr = NULL;

/// MMAP内容长度 指针
/// 位置不改变永远指向缓存文件 内容长度
int *mmap_content_len_ptr = NULL;

/// MMAP头部内容长度 指针
/// 位置不改变永远指向缓存文件 头部长度
int *mmap_header_content_len_ptr = NULL;

/// MMAP头部内容 指针
/// 位置不改变永远指向缓存文件 头部内容
char *mmap_header_content_ptr = NULL;

/// MMAP最后一条日志信息的长度指针
/// 位置变化，永远指向最后一条日志的内容长度，没有日志内容 则指向NULL
int *mmap_log_content_ptr = NULL;



/**
 读取缓存头文件信息
 文件内容长度 & 头信息内容
 
 @param mmap_buffer mmap 绑定指针 | 指向缓存文件开头
 @return 1成功 0失败
 */
int bind_cache_file_pointer_from_header(unsigned char *mmap_buffer) {
    
    unsigned char *temp = mmap_buffer;
    if (*temp == FD_MMAP_FILE_HEADER) {
        temp += 1;
        /// 读取头部信息长度
        mmap_header_content_len_ptr = (int*)temp;
        printf("mmap_header_content_len_ptr:%d\n",*mmap_header_content_len_ptr);
        temp += sizeof(int);
        /// 读取头部内容
        char *header_content = (char*)malloc(*mmap_header_content_len_ptr + 1);
        memset(header_content, '\0', *mmap_header_content_len_ptr);
        memcpy(header_content, temp, *mmap_header_content_len_ptr);
        
        temp += *mmap_header_content_len_ptr;
        printf("header_content: %s \n",header_content);
        if (*temp == FD_MMAP_FILE_TAILER) {
            printf("READ FD_MMAP_FILE_TAILER ✅\n");
            temp += 1;
            
            // 判断是否是 剩余数据的头部
            if (*temp == FD_MMAP_FILE_REMAIN_DATA_HEADER) {
                mmap_remain_ptr = temp;
                temp += 1;
                printf("READ FD_MMAP_FILE_REMAIN_DATA_HEADER ✅\n");
                temp += sizeof(int);
                temp += 16;
                if (!(*temp == FD_MMAP_FILE_REMAIN_DATA_TAILER)) {
                    printf("READ FD_MMAP_FILE_REMAIN_DATA_TAILER ❌\n");
                    return 0;
                }
                temp += 1;
            }
            
            if (*temp == FD_MMAP_LAST_LOG_CONTENT_LEN_DISTANCE_HEADER) {
                mmap_last_log_content_len_distance_ptr = temp;
                temp += 1;
                printf("READ FD_MMAP_LAST_LOG_CONTENT_LEN_DISTANCE_HEADER ✅\n");
                temp += sizeof(int);
                if (!(*temp == FD_MMAP_LAST_LOG_CONTENT_LEN_DISTANCE_TAILER)) {
                    printf("READ FD_MMAP_LAST_LOG_CONTENT_LEN_DISTANCE_TAILER ❌\n");
                    return 0;
                }
                temp += 1;
            }
            
            // 判断是否是 文件内容的数据的头部
            if (*temp == FD_MMAP_FILE_CONTENT_HEADER) {
                temp += 1;
                printf("READ FD_MMAP_FILE_CONTENT_HEADER ✅\n");
                temp += sizeof(int);
                
                if (*temp == FD_MMAP_FILE_CONTENT_TAILER) {
                    printf("READ FD_MMAP_FILE_CONTENT_TAILER ✅\n");
                    temp -= sizeof(int);
                    
                    /// 读取总内容长度
                    if (mmap_header_content_ptr != NULL) {
                        mmap_content_len_ptr = (int*)temp;
                        memcpy(mmap_header_content_ptr, header_content, *mmap_header_content_len_ptr);
//                        free(header_content);
//                        header_content = NULL;
                        
                        mmap_tailer_ptr = mmap_ptr;
                        int move_len = 2 + sizeof(int) + *mmap_header_content_len_ptr + 2 + sizeof(int) + 16 + 2 + sizeof(int) + *mmap_content_len_ptr;
                        printf("move_len:%d \n",move_len);
                        mmap_tailer_ptr += move_len;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}


int update_len_pointer(int increase_content_len) {
    if ((mmap_ptr != NULL) && (mmap_content_len_ptr != NULL) && (mmap_header_content_ptr != NULL) && (mmap_header_content_len_ptr != NULL) && (mmap_tailer_ptr != NULL)) {
        
        int ret = bind_cache_file_pointer_from_header(mmap_ptr);
        if (ret == 0) { return 0; }
        
        /// 更新内容长度
        int new_len = *mmap_content_len_ptr + increase_content_len;
        memcpy(mmap_content_len_ptr,&(new_len),sizeof(int));
        printf("mmap_content_len_ptr:%d \n",*mmap_content_len_ptr);

        /// 移动尾巴
        mmap_tailer_ptr += increase_content_len;
        return 1;
    }
    return 0;
}




