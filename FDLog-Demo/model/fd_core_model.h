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
#define FD_MAX_MMAP_SCALE 0.8  // 缓存文件的百分比 开始写入 本地文件

#define FD_DATE "fd_date"
#define FD_SIZE "fd_mmap_size"

//#define FD_MAX_GZIP_SIZE 1024*50
#define FD_MAX_LOG_SIZE 1024*1024*2 // 最大日志文件尺寸


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


/**
 日志核心处理模型
 */
typedef struct fd_core_model {
    
    /// FDLog: AES_IV
    unsigned char aes_iv[16];
    
    /// FDLog: 是否可以gzip
    int is_ready_gzip;
    
    /// FDLog: 压缩类型
    /// 定义Logan_zlib的状态类型
    /// #define FD_ZLIB_NONE 0
    /// #define FD_ZLIB_INIT 1
    /// #define FD_ZLIB_ING  2
    /// #define FD_ZLIB_END  3
    /// #define FD_ZLIB_FAIL 4
    int zlib_type;

    /// FDLog: 日志文件夹地址
    /// 0绑定失败 1绑定成功
    int is_bind_mmap;
    
    /// FDLog: 日志gzip压缩流对象
    /// 当使用zlib压缩的时候需要使用 z_stream 进行初始化
    z_stream *strm;
    
    /// FDLog: 剩余日志数据
    /// 由于使用 AES加密 数据块必须是16的倍数，所以存在剩余数据空间。
    char cache_remain_data[16];
    
    /// FDLog: 剩余空间长度
    int cache_remain_data_len;
    
} FDLOGMODEL;


int adjust_mmap_tailer(unsigned char *mmap_buffer);

int update_mmap_content_len(int increase_content_len);


#endif /* fd_core_model_h */
