//
//  fd_zlib_helper.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_zlib_helper.h"
#include "fd_aes_helper.h"
#include "fd_console_helper.h"
#include <zlib.h>
#include <stdlib.h>
#include <string.h>

extern FDLOGMODEL *model;

int fd_init_zlib() {
    if (!model->strm) {
        model->strm = calloc(1, sizeof(z_stream));
    }
    model->is_ready_gzip = 0;
    model->is_zlibing = 0;
    memset(model->strm, 0, sizeof(z_stream));
    model->strm->zalloc = Z_NULL;
    model->strm->zfree = Z_NULL;
    model->strm->opaque = Z_NULL;

    int ret1 = deflateInit2(model->strm, Z_BEST_COMPRESSION, Z_DEFLATED, (15 + 16), 8,
                            Z_DEFAULT_STRATEGY);
    if (ret1 == Z_OK) {
        model->is_ready_gzip = 1;
        fd_printf("deflateInit2 success! \n");
        return 1;
    }
    else {
        fd_printf("deflateInit2 failture! \n");
        return 0;
    }
}

int fd_zlib_compress(char *data, int data_len, int type) {
    if (model->is_zlibing) {
        fd_printf("FDLog fd_zlib_compress: fdlog writing cache, can't write again! \n");
        return 0;
    }
    else {
        model->is_zlibing = 1;
    }
    
    int is_gzip = model->is_ready_gzip;
    int ret;
    if (is_gzip) {
        unsigned int have;
        unsigned char out[LOGAN_CHUNK];
        z_stream *strm = model->strm;
        strm->avail_in = (uInt) data_len;
        strm->next_in = (unsigned char *) data;
        
        do {
            strm->avail_out = LOGAN_CHUNK;
            strm->next_out = (unsigned char *) out;
            ret = deflate(strm, type);
            if (Z_STREAM_ERROR == ret) {
                deflateEnd(model->strm);
                model->is_ready_gzip = 0;
            } else {
                have = LOGAN_CHUNK - strm->avail_out;
                int total_len = model->cache_remain_data_len + have;
                unsigned char *temp = NULL;
                
                int handler_len = (total_len / 16) * 16;
                int remain_len = total_len % 16;
                
                if (handler_len) {
                    int copy_data_len = handler_len - model->cache_remain_data_len;
                    char gzip_data[handler_len];
                    temp = (unsigned char *) gzip_data;
                    
                    if (model->cache_remain_data_len) {
                        memcpy(temp, model->cache_remain_data, model->cache_remain_data_len);
                        temp += model->cache_remain_data_len;
                    }
                    
                    memcpy(temp, out, copy_data_len);
                    
                    fd_aes_encrypt((unsigned char *) gzip_data, model->mmap_tailer_ptr, handler_len,
                                   (unsigned char *) model->aes_iv);

                    *model->mmap_current_log_len_ptr += handler_len;
                    model->mmap_tailer_ptr += handler_len;
                }
                if (remain_len) {
                    if (handler_len) {
                        int copy_data_len = handler_len - model->cache_remain_data_len;
                        temp = (unsigned char *) out;
                        temp += copy_data_len;
                        memcpy(model->cache_remain_data, temp, remain_len);
                    } else {
                        temp = (unsigned char *) model->cache_remain_data;
                        temp += model->cache_remain_data_len;
                        memcpy(temp, out, have);
                    }
                }
                model->cache_remain_data_len = remain_len;
            }
        } while (strm->avail_out == 0);
        return 1;
    } else {
        model->is_zlibing = 0;
        return 0;
    }
}

void fd_zlib_end_compress() {
    fd_zlib_compress(NULL, 0, Z_FINISH);
    (void) deflateEnd(model->strm);
    
    int val = 16 - model->cache_remain_data_len;
    char data[16];
    memset(data, val, 16);
    
    if (model->cache_remain_data_len) {
        memcpy(data, model->cache_remain_data, model->cache_remain_data_len);
    }
    
    fd_aes_encrypt((unsigned char *) data, model->mmap_tailer_ptr, 16,
                   (unsigned char *) model->aes_iv);
    
    model->mmap_tailer_ptr += 16;
    *(model->mmap_tailer_ptr) = FD_MMAP_WRITE_CONTENT_TAILER;
    model->mmap_tailer_ptr++;
    *model->mmap_current_log_len_ptr += 16;
    
    *model->mmap_content_len_ptr += (*model->mmap_current_log_len_ptr + 2 + sizeof(int));

    model->cache_remain_data_len = 0;
    model->is_ready_gzip = 0;
    model->is_zlibing = 0;
}

