//
//  fd_log.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_log_h
#define fd_log_h
#include "fd_construct_data.h"

/////////////////////////////////////////////
////       FDLog 日志 API V1.0.0          ////
/////////////////////////////////////////////

/**
 日志初始化

 Usage:
 // 当前地址
 char cwd[1024];
 if (getcwd(cwd, sizeof(cwd)) != NULL) {
 printf("Current working dir: %s \n", cwd);
 } else {
 perror("getcwd() error \n");
 return 1;
 }
 
 // 开始记录日志
 char *log = "FDLog 日志记录！";
 int flag = 5;
 long long localtime = 123123;
 char thread_name[] = "main";
 int thread_id = 1;
 int is_main = 1;
 FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main);
 
 char KEY[] = "0123456789012345";
 char IV[] = "0123456789012345";
 int success = fdlog_initialize(cwd, KEY, IV);
 if (!success) {
 printf("fd_initialize_log failture! \n");
 }
 
 @param root 日志根目录
 @param key AES128 KEY (加密方式用的是AES128 PADDING模式PKCS7 CBC模式)
 @param iv AES128 IV (加密方式用的是AES128 PADDING模式PKCS7 CBC模式)
 @return 1成功 0失败
 */
int fdlog_initialize(char* root, char* key, char* iv);

/**
 写日志

 需要 fdlog_initialize 成功后才可以调用。
 
 Usage:
 // 开始记录日志
 char *log = "FDLog 日志记录！";
 int flag = 5;
 long long localtime = 123123;
 char thread_name[] = "main";
 int thread_id = 1;
 int is_main = 1;
 FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main);
 fdlog(data);
 
 @param data 日志信息数据结构
 @return 1成功 0失败
 */
int fdlog(FD_Construct_Data *data);

/**
 同步缓存到磁盘

 需要 fdlog_initialize 成功后才可以调用。
 强制将缓存的日志同步的本地日志文件中，无视一切规则。
 
 @return 1成功 0失败
 */
int fdlog_sync(void);

#endif /* fd_log_h */

