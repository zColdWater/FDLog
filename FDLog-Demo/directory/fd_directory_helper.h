#ifndef fd_directory_helper_h
#define fd_directory_helper_h

/**
 创建文件夹

 @param path 文件路径
 @return 是否创建成功
 */
int fd_makedir(const char *path);

/**
 文件是否存在

 @param path 文件路径
 @return 是否存在
 */
int fd_is_file_exist(const char *path);

long getmaximum(long a[], int numberOfElements);
char* get_current_date(void);
char* look_for_last_logfile(void);
int create_new_logfile(void);


#endif
