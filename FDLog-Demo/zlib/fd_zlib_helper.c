//
//  fd_zlib_helper.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_zlib_helper.h"
#include "fd_aes_helper.h"

/**
 压缩

 @param model logmodel
 @param data 压缩字节
 @param data_len 压缩字节长度
 @param type 压缩种类
 @return 是否成功
 */
bool fd_zlib(fd_logmodel *model, char *data, int data_len, int type) {
    
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
                fd_zlib_delete_stream(model);
            } else {
                // 压缩字节长度
                have = LOGAN_CHUNK - strm->avail_out;
                // 总长度 = 剩余的数据长度 + 本次压缩的数据长度
                int total_len = model->remain_data_len + have;
                unsigned char *temp = NULL;
                
                // 处理字节长度 由于AES 只处理16的整数倍
                int handler_len = (total_len / 16) * 16;
                // 剩余处理的字节长度
                int remain_len = total_len % 16;
                
                // 如果存在可以处理的字节长度
                if (handler_len) {
                    // 拿到本次要处理的字节长度 = 总字节长度 - 上次剩下的字节长度
                    int copy_data_len = handler_len - model->remain_data_len;
                    // 声明gzip_data字符串数组大小
                    char gzip_data[handler_len];
                    // temp指向gzip的首地址
                    temp = (unsigned char *) gzip_data;
                    
                    // 如果存在上次剩余的数据长度
                    if (model->remain_data_len) {
                        // 将上次的数据copy到temp数组中，并且通过指针运算移动到最后。
                        memcpy(temp, model->remain_data, model->remain_data_len);
                        temp += model->remain_data_len;
                    }
                    
                    // 将本次压缩的数据 也放入 temp中
                    memcpy(temp, out, copy_data_len);
                    
                    // 开始进行AES加密操作 并且 当执行完加密操作 也把数据写入了缓存
                    fd_aes_encrypt((unsigned char *) gzip_data, model->last_point, handler_len,
                                       (unsigned char *) model->aes_iv);
                    // 设置logmodel总长度
                    model->total_len += handler_len;
                    // 设置logmodel内容长度
                    model->content_len += handler_len;
                    // 设置logmodel最后指针指向
                    model->last_point += handler_len;
                }
                
                // 如果本次压缩存在剩余数据长度
                if (remain_len) {
                    // 本次压缩操作 + 上次剩余 满足16的整数倍
                    if (handler_len) {
                        
                        // 计算出剩余数据长度，并且给 model->remain_data赋值
                        int copy_data_len = handler_len - model->remain_data_len;
                        temp = (unsigned char *) out;
                        temp += copy_data_len;
                        memcpy(model->remain_data, temp, remain_len);
                    } else { // 本次压缩操作 + 上次剩余 没有满足16的整数倍
                        
                        // remain_data 增加长度 留作下次使用费
                        temp = (unsigned char *) model->remain_data;
                        temp += model->remain_data_len;
                        memcpy(temp, out, have);
                    }
                }
                
                // 更新剩余数据长度
                model->remain_data_len = remain_len;
            }
        } while (strm->avail_out == 0); // avail_out， next_out还有多少字节空间可以用来保存输出字节
        
        return true;
    } else {
        return false;
    }
    
//    else { // 不能压缩成gzip
//
//        // 跳过压缩步骤 其余与上面一致
//        int total_len = model->remain_data_len + data_len;
//        unsigned char *temp = NULL;
//        int handler_len = (total_len / 16) * 16;
//        int remain_len = total_len % 16;
//        if (handler_len) {
//            int copy_data_len = handler_len - model->remain_data_len;
//            char gzip_data[handler_len];
//            temp = (unsigned char *) gzip_data;
//            if (model->remain_data_len) {
//                memcpy(temp, model->remain_data, model->remain_data_len);
//                temp += model->remain_data_len;
//            }
//            memcpy(temp, data, copy_data_len); //填充剩余数据和压缩数据
//
//            fd_aes_encrypt((unsigned char *) gzip_data, model->last_point, handler_len,
//                               (unsigned char *) model->aes_iv);
//            model->total_len += handler_len;
//            model->content_len += handler_len;
//            model->last_point += handler_len;
//        }
//        if (remain_len) {
//            if (handler_len) {
//                int copy_data_len = handler_len - model->remain_data_len;
//                temp = (unsigned char *) data;
//                temp += copy_data_len;
//                memcpy(model->remain_data, temp, remain_len); //填充剩余数据和压缩数据
//            } else {
//                temp = (unsigned char *) model->remain_data;
//                temp += model->remain_data_len;
//                memcpy(temp, data, data_len);
//            }
//        }
//        model->remain_data_len = remain_len;
//    }
}

bool fd_init_zlib(fd_logmodel *model) {
    bool ret = false;
    // 如果是init的状态则不需要init
    if (model->zlib_type == FD_ZLIB_INIT) {
        return true;
    }
    // 声明 zlib z_stream 压缩流指针
    z_stream *temp_zlib = NULL;
    // 当logmodel没有申请zlib内存时
    if (!model->is_malloc_zlib) {
        // malloc 堆内存给 压缩流指针
        temp_zlib = malloc(sizeof(z_stream));
    } else {
        // 将logmodel的压缩流指针给临时压缩流指针指向
        temp_zlib = model->strm;
    }
    // ZLIB初始化失败
    if (temp_zlib == NULL) {
        model->is_malloc_zlib = NOT_MALLOC_ZLIB_STREAM;
        model->is_ready_gzip = NOT_READY_GZIP;
        model->zlib_type = FD_ZLIB_FAIL;
    }
    else {
        model->is_malloc_zlib = MALLOC_ZLIB_STREAM; //表示已经 malloc 一个zlib
        memset(temp_zlib, 0, sizeof(z_stream));
        model->strm = temp_zlib;
        
        temp_zlib->zalloc = Z_NULL;
        temp_zlib->zfree = Z_NULL;
        temp_zlib->opaque = Z_NULL;
        
        // 初始化zlib，具体参数 参考API
        int ret1 = deflateInit2(temp_zlib, Z_BEST_COMPRESSION, Z_DEFLATED, (15 + 16), 8,
                                Z_DEFAULT_STRATEGY);
        
        if (ret1 == Z_OK) { // 初始化成功
            ret = true;
            model->is_ready_gzip = READY_GZIP;
            model->zlib_type = FD_ZLIB_INIT; // 状态变成 ZLIB成功初始化
        } else { // 初始化失败
            ret = false;
            model->is_ready_gzip = NOT_READY_GZIP;
            model->zlib_type = FD_ZLIB_FAIL; // 状态变成 ZLIB初始化失败
        }
    }
    return ret;
}

void fd_zlib_end_compress(fd_logmodel *model) {
    // 完成压缩
    fd_zlib(model, NULL, 0, Z_FINISH);
    // 压缩完成以后,释放空间,但是注意,仅仅是释放deflateInit中申请的空间,自己申请的空间还是需要自己释放
    (void) deflateEnd(model->strm);
    // 算出和16字节差多少
    int val = 16 - model->remain_data_len;
    // 声明一个16字节的数组
    char data[16];
    // 用差值填满这 data 16字节
    memset(data, val, 16);
    
    // 如果存在剩余数据
    if (model->remain_data_len) {
        //将剩余的数组写入data数组
        memcpy(data, model->remain_data, model->remain_data_len);
    }
    
    // 开始AES加密 并且 写入缓存
    fd_aes_encrypt((unsigned char *) data, model->last_point, 16,
                       (unsigned char *) model->aes_iv);
    // 移动尾部指针到最尾一位
    model->last_point += 16;
    // 加入写入结尾
    *(model->last_point) = FD_WRITE_PROTOCOL_TAIL;
    // 移动尾部指针
    model->last_point++;
    
    // 重置剩余数据长度为0
    model->remain_data_len = 0;
    // 总长度 + 17（结尾数据补全 + 写入协议尾部）
    model->total_len += 17;
    //为了兼容之前协议content_len,只包含内容,不包含结尾符
    model->content_len += 16;
    
    // 改变 Zlib 状态
    model->zlib_type = FD_ZLIB_END;
    // 改变 is_ready_gzip 状态
    model->is_ready_gzip = NOT_READY_GZIP;
}

void fd_zlib_compress(fd_logmodel *model, char *data, int data_len) {
    if (model->zlib_type == FD_ZLIB_ING || model->zlib_type == FD_ZLIB_INIT) {
        // 改变zlib状态
        model->zlib_type = FD_ZLIB_ING;
        // 开始zlib压缩
        fd_zlib(model, data, data_len, Z_SYNC_FLUSH);
    } else {
        // 初始化zlib
        fd_init_zlib(model);
    }
}

void fd_zlib_delete_stream(fd_logmodel *model) {
    // 压缩完成以后,释放空间,但是注意,仅仅是释放deflateInit中申请的空间,自己申请的空间还是需要自己释放
    deflateEnd(model->strm);
    // 重新设置 ready_gzip 状态
    model->is_ready_gzip = NOT_READY_GZIP;
    // 重新设置 zlib_type 状态
    model->zlib_type = FD_ZLIB_END;
}
