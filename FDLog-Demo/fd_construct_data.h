#ifndef fd_construct_data_h
#define fd_construct_data_h

typedef struct {
    char *data; // log data
    int data_len; // log data length
} FD_Construct_Data;


/*
 * Function: fd_construct_json_data
 * ----------------------------
 *   Returns FD_Construct_Data
 *
 *   log: log content data
 *   flag: log flag type
 *   local_time: record log time
 *   thread_name: thread name
 *   thread_id: thread id
 *   is_main: is main thread
 *
 *   returns: FD_Construct_Data data
 */
FD_Construct_Data *
fd_construct_json_data(char *log, int flag, long long local_time, char *thread_name,
                           long long thread_id, int is_main, int level);


/*
 * Function: fd_construct_data_delete
 * ----------------------------
 *
 *   data: FD_Construct_Data
 *
 */
void fd_construct_data_delete(FD_Construct_Data *data);

#endif
