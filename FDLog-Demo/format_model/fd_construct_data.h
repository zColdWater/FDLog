#ifndef fd_construct_data_h
#define fd_construct_data_h


/**
 日志信息模型
 */
typedef struct {
    char *data; // 日志内容
    int data_len; // 日志长度
} FD_Construct_Data;

FD_Construct_Data *
fd_construct_json_data(char *log, int flag, long long local_time, char *thread_name,
                           long long thread_id, int is_main);

void fd_construct_data_delete(FD_Construct_Data *data);

#endif
