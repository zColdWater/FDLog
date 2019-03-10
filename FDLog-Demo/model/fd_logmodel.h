//
//  fd_logmodel.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_logmodel_h
#define fd_logmodel_h

#include <zlib.h>
#include <stdio.h>


#define FD_VERSION_KEY "logan_version"
#define FD_PATH_KEY "file"

#define FD_WRITE_PROTOCOL_HEADER '\1' // 写入协议头
#define FD_WRITE_PROTOCOL_TAIL '\0' // 写入协议尾

#define FD_CACHE_DIR "logan_cache"
#define FD_CACHE_FILE "logan.mmap2"

#define FD_MMAP_HEADER_PROTOCOL '\15' //MMAP的头文件标识符
#define FD_MMAP_TAIL_PROTOCOL '\16' //MMAP尾文件标识符
#define FD_MMAP_TOTALLEN  3 //MMAP文件长度

#define FD_MAX_GZIP_UTIL 5 * 1024 //压缩单元的大小 5KB

#define FD_WRITEPROTOCOL_HEAER_LENGTH 5 //Logan写入协议的头和写入数据的总长度

#define FD_WRITEPROTOCOL_DEVIDE_VALUE 3 //多少分之一写入

#define FD_DIVIDE_SYMBOL "/" // 拼接路径地址

#define FD_LOGFILE_MAXLENGTH 10485760 //10*1024*1024 // 文件最大长度 10MB

#define FD_WRITE_SECTION 20 * 1024 //多大长度做分片 大于20K做分片

#define FD_RETURN_SYMBOL "\n"

#define FD_FILE_NONE 0  // 没有文件
#define FD_FILE_OPEN 1 // 文件流开启
#define FD_FILE_CLOSE 2 // 文件流关闭

#define FD_EMPTY_FILE 0 // 文件为空

#define FD_VERSION_NUMBER 3 //Logan的版本号(2)版本




#define MALLOC_ZLIB_STREAM 1 // 已经创建 z_stream 并且赋值给 logmodel.strm [fd_logmodel.is_malloc_zlib]
#define NOT_MALLOC_ZLIB_STREAM 0 // 没有创建 z_stream 并且赋值给 [fd_logmodel.is_malloc_zlib]

#define READY_GZIP 1 // 可以压缩GZIP [fd_logmodel.is_ready_gzip]
#define NOT_READY_GZIP 0 // 不可以压缩GZIP [fd_logmodel.is_ready_gzip]



/**
 核心日志模型
 */
typedef struct fd_logmodel {
    int total_len; //数据长度
    char *file_path; //文件路径
    
    int is_malloc_zlib;
    z_stream *strm;
    int zlib_type; //压缩类型
    char remain_data[16]; //剩余空间 被压缩过的
    int remain_data_len; //剩余空间长度
    
    int is_ready_gzip; //是否可以gzip
    
    int file_stream_type; //文件流类型
    FILE *file; //文件流
    
    long file_len; //文件大小
    
    unsigned char *buffer_point; //缓存的指针 (不变)
    unsigned char *last_point; //最后写入位置的指针
    unsigned char *total_point; //总数的指针 (可能变) , 给c看,低字节
    unsigned char *content_lent_point;//协议内容长度指针 , 给java看,高字节
    int content_len; //内容的大小
    
    unsigned char aes_iv[16]; //aes_iv
    int is_ok;
    
} fd_logmodel;
#endif /* fd_logmodel_h */
