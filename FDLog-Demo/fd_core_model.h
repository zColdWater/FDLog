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
*/

#define FD_VERSION_KEY "fdlog_version"
#define FD_VERSION_NUMBER "1" // FDLog 版本号 (每条日志都带有FDLog版本)
#define FD_LOG_FOLDER_NAME "fdlog_v" FD_VERSION_NUMBER
#define FD_LOG_CACHE_FOLDER_NAME "cache"
#define FD_LOG_FILE_FOLDER_NAME "logs"

#define FD_LOG_CACHE_NAME "cache.mmap"
#define FD_DATE "fd_date"
#define FD_SERVER_VER "fd_server_ver" // 服务器下发 AES(Key & IV) 的版本
#define FD_SIZE "fd_mmap_size"

#define FD_AES_IV "IV" // AES 加密 IV （ 解析服务器下发RSA加密数据中的IV字段 ）
#define FD_AES_KEY "key" // AES 加密 KEY （ 解析服务器下发RSA加密数据中的key字段 ）
#define FD_SERVER_VER1 "version" // 服务器版本 （ 解析服务器下发RSA加密数据中的version字段 ）


#define FD_MAX_PATH 1024 * 3
#define FD_MAX_FILE_COUNT 1024 * 2 // 当天最多日志个数 2048个
#define FD_MMAP_HEADER_CONTENT_LEN 1024 // 缓存MMAP文件头部信息 存储大小，目前1024完全够用，如果修改插入头部内容多少需要修改此处值。
#define FD_MAX_LOG_LENGTH 110000 // 单条日志最大输入字节要小于110000。

/* 当缓存内容占整个文件到这个比例时 会输出到日志文件中 */
#define FD_MAX_MMAP_SCALE 0.3

/* 设置日志文件最大Size */
#define FD_MAX_LOG_SIZE 500 * 1024 // 1024*1024*5 最大5M，否则8M以上会导致越界。 默认400KB大小 1份日志文件

/* 设置默认上传最近7天的日志 */
#define FD_SAVE_RECENT_DAYS 7

/* CACHE LOG HEAD PROTOCOL  */
#define FD_MMAP_FILE_HEADER '#'
#define FD_MMAP_FILE_TAILER '$'

/* CACHE LOG TOTAL LEN PROTOCOL  */
#define FD_MMAP_TOTAL_LOG_LEN_HEADER '%'
#define FD_MMAP_TOTAL_LOG_LEN_TAILER '&'

/* CACHE SINGLE LOG PROTOCOL  */
#define FD_MMAP_WRITE_CONTENT_HEADER '!'
#define FD_MMAP_WRITE_CONTENT_TAILER '@'


typedef struct fd_core_model {
    
    int is_bind_mmap; // 0 or 1
    int is_ready_gzip; // 0 or 1
    int is_init_global_vars; // 0 or 1
    int is_zlibing; // 0 or 1 是否正在压缩状态
    int server_ver; // 服务器的版本号 其实就是 AES（KEY 和 IV）对应的版本
    
    unsigned int save_recent_days_num; // 保存最近多少天的日志
    unsigned long long max_logfix_size; // 日志文件最大尺寸

    unsigned char aes_iv[16];
    z_stream *strm;
    char cache_remain_data[16];
    int cache_remain_data_len;
    
    unsigned char *mmap_ptr; // 指向缓存文件头 位置不变
    unsigned char *mmap_tailer_ptr; // 指向缓存文件尾 位置不断变化随着写入而变化
    int *mmap_content_len_ptr; // 缓存文件里日志总量长度 位置不变
    int *mmap_current_log_len_ptr; // 最后一条日志的长度 位置不断变化随着写入而变化
    int *mmap_header_content_len_ptr; // 缓存文件 头部 内容长度 位置不变
    char *mmap_header_content_ptr; // 存储 缓存文件 头部内容 (malloc)
    
    char *log_folder_path; // 日志文件夹地址
    long *log_file_len; // 日志文件字节长度
    char *log_file_path; // 日志文件地址
    char *mmap_cache_file_path; // 缓存文件路径
    
} FDLOGMODEL;


#endif /* fd_core_model_h */
