//
//  fd_core_model.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/3/5.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_core_model_h
#define fd_core_model_h

#include <zlib.h>
#include <stdio.h>


/** 日志缓存结构
|缓存文件头部|头部信息|缓存文件的尾部|
|缓存文件总内容长度头部|4字节存储总长度|缓存文件总内容长度尾部|
|缓存日志写入头|4字节本条日志长度|日志内容|缓存日志写入尾|
|缓存日志写入头|4字节本条日志长度|日志内容|缓存日志写入尾|
|缓存日志写入头|4字节本条日志长度|日志内容|缓存日志写入尾|
|缓存日志写入头|4字节本条日志长度|日志内容|缓存日志写入尾|
|缓存日志写入头|4字节本条日志长度|日志内容|缓存日志写入尾|
*/

#define FD_VERSION_KEY "fdlog_version"
#define FD_VERSION_NUMBER 1 // FDLog 版本号
#define FD_VERSION "v1"
#define FD_LOG_FOLDER_NAME "fdlog_" FD_VERSION
#define FD_LOG_CACHE_FOLDER_NAME "cache"
#define FD_LOG_CACHE_NAME "cache.mmap"
#define FD_MAX_PATH 1024 * 3
#define FD_MMAP_HEADER_CONTENT_LEN 1024
#define FD_MAX_MMAP_SCALE 0.3  // 缓存文件的百分比 开始写入 本地文件

#define FD_DATE "fd_date"
#define FD_SIZE "fd_mmap_size"

//#define FD_MAX_GZIP_SIZE 1024*50
#define FD_MAX_LOG_SIZE 300 * 1024 //1024*1024*2 // 最大日志文件尺寸


/// 缓存文件的头部的头字符
#define FD_MMAP_FILE_HEADER '#'
/// 缓存文件的头部的尾字符
#define FD_MMAP_FILE_TAILER '$'

/// 缓存文件的日志总内容长度头
#define FD_MMAP_FILE_CONTENT_HEADER '%'
/// 缓存文件的日志总内容长度尾
#define FD_MMAP_FILE_CONTENT_TAILER '&'

/// 缓存内容 写入头
#define FD_MMAP_FILE_CONTENT_WRITE_HEADER '!'
/// 缓存内容 写入尾
#define FD_MMAP_FILE_CONTENT_WRITE_TAILER '@'


#define READY_GZIP 1 // 可以压缩GZIP [fd_logmodel.is_ready_gzip]
#define NOT_READY_GZIP 0 // 不可以压缩GZIP [fd_logmodel.is_ready_gzip]


typedef struct fd_core_model {
    
    int is_bind_mmap; // 0 or 1
    int is_ready_gzip; // 0 or 1
    int is_init_global_vars; // 0 or 1
    int is_zlibing; // 0 or 1 是否正在压缩状态
    
    unsigned char aes_iv[16];
    int zlib_type;
    z_stream *strm;
    char cache_remain_data[16];
    int cache_remain_data_len;
    
} FDLOGMODEL;


int update_mmap_content_len(int increase_content_len);


#endif /* fd_core_model_h */
