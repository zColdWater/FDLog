//
//  fd_zlib_helper.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_zlib_helper.h"
#include "fd_aes_helper.h"
#include <zlib.h>
#include <stdlib.h>
#include <string.h>


extern FDLOGMODEL *model;
extern unsigned char *mmap_ptr;
extern unsigned char *mmap_tailer_ptr;
extern int *mmap_content_len_ptr;
extern int *mmap_header_content_len_ptr;
extern char *mmap_header_content_ptr;
extern int *mmap_log_content_ptr;


bool fd_init_zlib(FDLOGMODEL *model) {
    
    /// gzip 默认 false
    model->is_ready_gzip = false;
    /// zlib 默认 false
    model->zlib_type = FD_ZLIB_FAIL;

    if (model->strm != NULL) {
//        free(m->strm);
        model->strm = NULL;
    }
    
    model->strm = (z_stream *) malloc(sizeof(z_stream));
    if (model == NULL) {
        printf("m->strm = (z_stream *) malloc(sizeof(z_stream)) failture \n");
        return false;
    }
    memset(model->strm, 0, sizeof(z_stream));

    model->strm->zalloc = Z_NULL;
    model->strm->zfree = Z_NULL;
    model->strm->opaque = Z_NULL;

    // 初始化zlib，具体参数 参考API
    int ret1 = deflateInit2(model->strm, Z_BEST_COMPRESSION, Z_DEFLATED, (15 + 16), 8,
                            Z_DEFAULT_STRATEGY);
    
    if (ret1 == Z_OK) {
        model->is_ready_gzip = true;
        model->zlib_type = FD_ZLIB_INIT;
        printf("deflateInit2 success! \n");
        return true;
    }
    else {
        printf("deflateInit2 failture! \n");
        return false;
    }
}

bool fd_zlib_compress(FDLOGMODEL *model, char *data, int data_len, int type) {
    
    int is_gzip = model->is_ready_gzip;
    int ret;
    
    // 如果可以压缩成 gzip
    if (is_gzip) {
        unsigned int have;
        // 压缩后结果
        unsigned char out[LOGAN_CHUNK];
        // 获取压缩流
        z_stream *strm = model->strm;
        // 设置待压缩字符串长度
        strm->avail_in = (uInt) data_len;
        // 设置待压缩字符串指针
        strm->next_in = (unsigned char *) data;
        
        do {
            strm->avail_out = LOGAN_CHUNK;
            strm->next_out = (unsigned char *) out;
            // 执行压缩操作
            ret = deflate(strm, type);
            // 应用程序必须确保deflate()可以从next_in中读到数据，或者从next_out中写入数据，否则会返回Z_STREAM_ERROR
            if (Z_STREAM_ERROR == ret) {
                // 压缩完成以后,释放空间,但是注意,仅仅是释放deflateInit中申请的空间,自己申请的空间还是需要自己释放
                deflateEnd(model->strm);
                // 重新设置 ready_gzip 状态
                model->is_ready_gzip = NOT_READY_GZIP;
                // 重新设置 zlib_type 状态
                model->zlib_type = FD_ZLIB_END;
            } else {
                // 压缩字节长度
                have = LOGAN_CHUNK - strm->avail_out;
                // 总长度 = 剩余的数据长度 + 本次压缩的数据长度
                int total_len = model->cache_remain_data_len + have;
                unsigned char *temp = NULL;
                
                // 处理字节长度 由于AES 只处理16的整数倍
                int handler_len = (total_len / 16) * 16;
                // 剩余处理的字节长度
                int remain_len = total_len % 16;
                
                // 如果存在可以处理的字节长度
                if (handler_len) {
                    // 拿到本次要处理的字节长度 = 总字节长度 - 上次剩下的字节长度
                    int copy_data_len = handler_len - model->cache_remain_data_len;
                    // 声明gzip_data字符串数组大小
                    char gzip_data[handler_len];
                    // temp指向gzip的首地址
                    temp = (unsigned char *) gzip_data;
                    
                    // 如果存在上次剩余的数据长度
                    if (model->cache_remain_data_len) {
                        // 将上次的数据copy到temp数组中，并且通过指针运算移动到最后。
                        memcpy(temp, model->cache_remain_data, model->cache_remain_data_len);
                        temp += model->cache_remain_data_len;
                    }
                    
                    // 将本次压缩的数据 也放入 temp中
                    memcpy(temp, out, copy_data_len);
                    
                    // 开始进行AES加密操作 并且 当执行完加密操作 也把数据写入了缓存
                    fd_aes_encrypt((unsigned char *) gzip_data, mmap_tailer_ptr, handler_len,
                                   (unsigned char *) model->aes_iv);
                    
                    /// 更新最后一条未写完日志 内容长度。
                    int total_handler_len = *mmap_log_content_ptr + handler_len;
                    memcpy(mmap_log_content_ptr, &total_handler_len, sizeof(int));
                    /// 更新长度 和 重定位尾指针
                    update_len_pointer(handler_len);
                    
                }
                
                // 如果本次压缩存在剩余数据长度
                if (remain_len) {
                    // 本次压缩操作 + 上次剩余 满足16的整数倍
                    if (handler_len) {
                        
                        // 计算出剩余数据长度，并且给 model->remain_data赋值
                        int copy_data_len = handler_len - model->cache_remain_data_len;
                        temp = (unsigned char *) out;
                        temp += copy_data_len;
                        memcpy(model->cache_remain_data, temp, remain_len);
                    } else { // 本次压缩操作 + 上次剩余 没有满足16的整数倍
                        
                        // remain_data 增加长度 留作下次使用费
                        temp = (unsigned char *) model->cache_remain_data;
                        temp += model->cache_remain_data_len;
                        memcpy(temp, out, have);
                    }
                }
                
                // 更新剩余数据长度
                model->cache_remain_data_len = remain_len;
            }
        } while (strm->avail_out == 0); // avail_out， next_out还有多少字节空间可以用来保存输出字节
        
        return true;
        
    } else {
        
        return false;
        
    }
}


void fd_zlib_end_compress(FDLOGMODEL *model) {
    // 完成压缩
    fd_zlib_compress(model, NULL, 0, Z_FINISH);
    // 压缩完成以后,释放空间,但是注意,仅仅是释放deflateInit中申请的空间,自己申请的空间还是需要自己释放
    (void) deflateEnd(model->strm);
    // 算出和16字节差多少
    int val = 16 - model->cache_remain_data_len;
    // 声明一个16字节的数组
    char data[16];
    // 用差值填满这 data 16字节
    memset(data, val, 16);
    
    // 如果存在剩余数据
    if (model->cache_remain_data_len) {
        //将剩余的数组写入data数组
        memcpy(data, model->cache_remain_data, model->cache_remain_data_len);
    }
    
    // 开始AES加密 并且 写入缓存
    fd_aes_encrypt((unsigned char *) data, mmap_tailer_ptr, 16,
                   (unsigned char *) model->aes_iv);
    
    /// 更新最后一条未写完日志 内容长度。
    int total_handler_len = *mmap_log_content_ptr + 16;
    memcpy(mmap_log_content_ptr, &total_handler_len, sizeof(int));
    update_len_pointer(16);
    
    // 重置剩余数据长度为0
    model->cache_remain_data_len = 0;
    // 改变 Zlib 状态
    model->zlib_type = FD_ZLIB_END;
    // 改变 is_ready_gzip 状态
    model->is_ready_gzip = NOT_READY_GZIP;
}

