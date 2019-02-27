#ifndef fd_construct_data_h
#define fd_construct_data_h

/**
 日志信息模型
 */
typedef struct {
    char *data; // 日志内容
    int data_len; // 日志长度
} FD_Construct_Data;

/**
 生成日志format模型

 @param log 日志内容
 @param flag 类型
 @param local_time 系统当前时间
 @param thread_name 线程名字
 @param thread_id 线程id
 @param is_main 是否主线程
 @return FD_Construct_Data Object
 */
FD_Construct_Data *
fd_construct_json_data(char *log, int flag, long long local_time, char *thread_name,
                           long long thread_id, int is_main);

/**
 释放 FD_Construct_Data Object

 @param data 日志format模型
 */
void fd_construct_data_delete(FD_Construct_Data *data);

#endif
