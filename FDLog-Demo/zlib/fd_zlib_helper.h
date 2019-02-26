//
//  fd_zlib_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_zlib_helper_h
#define fd_zlib_helper_h

#include "fd_logmodel.h"
#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 定义数据块大小
#define LOGAN_CHUNK 16384 // 16KB

// 定义Logan_zlib的状态类型
#define FD_ZLIB_NONE 0
#define FD_ZLIB_INIT 1
#define FD_ZLIB_ING  2
#define FD_ZLIB_END  3
#define FD_ZLIB_FAIL 4


/**
 初始化zlib

 初始化zlib，并且修改log模型里面的关于zlib相关属性
 
 @param model log模型
 @return bool 初始化是否成功
 */
bool fd_init_zlib(fd_logmodel *model);


/**
 开始压缩

 @param model logmodel
 @param data 待压缩的字节
 @param data_len 待压缩的字节长度
 */
void fd_zlib_compress(fd_logmodel *model, char *data, int data_len);


/**
 结束压缩
 
 将剩余数据全部压缩 也是z_stream最后一次压缩

 @param model logmodel
 */
void fd_zlib_end_compress(fd_logmodel *model);


/**
 释放压缩流
 
 修正压缩相关状态 删除初始化的z_stream

 @param model logmodel
 */
void fd_zlib_delete_stream(fd_logmodel *model);

#endif /* fd_zlib_helper_h */
