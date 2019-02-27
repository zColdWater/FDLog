//
//  fd_log.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_log_h
#define fd_log_h

#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include "fd_logmodel.h"


/////////////////////////////////////////////
////       FDLog 日志 API V1.0.0          ////
/////////////////////////////////////////////

/**
 FDLog: 初始化FDLog
 1.确定缓存文件类型
 2.如果缓存文件有内容，创建本地日志文件，并且将缓存写入本地。
 3.确定AES加密 Key 和 IV

 @param cache_dirs 日志缓存文件夹路径
 @param path_dirs 日志本地文件夹路径
 @param max_file 文件最大长度
 @param encrypt_key16 16位Key
 @param encrypt_iv16 16位向量
 @return 初始化状态
 */
int fdlog_init(const char *cache_dirs,
               const char *path_dirs,
               int max_file,
               const char *encrypt_key16,
               const char *encrypt_iv16);

/**
 创建本地日志文件

 @param pathname 文件名字，一般以时间为主
 @return 打开文件的状态
 */
int fdlog_open(const char *pathname);

/**
 写入日志
 先写入缓存文件，当缓存满足条件存入本地文件。

 @param flag 类型
 @param log 日志内容
 @param local_time 系统时间
 @param thread_name 线程名字
 @param thread_id 线程id
 @param is_main 是否是主线程
 @return 是否写入成功
 */
int fdlog_write(int flag,
                char *log,
                long long local_time,
                char *thread_name,
                long long thread_id,
                int is_main);

/**
 缓存文件写入本地日志文件

 @return 是否成功写入
 */
int fdlog_flush(void);

/**
 FDLog: 开启DEBUG模式 输出
 是否为debug环境 1为开启 0 为关闭
 
 @param debug 是否开启
 */
void fdlog_debug(int debug);

#endif /* fd_log_h */

