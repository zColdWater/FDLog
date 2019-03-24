#ifndef fd_base_helper_h
#define fd_base_helper_h

/**
 获取当前系统时间

 @return 系统时间
 */
long long fd_get_system_current(void);

/**
 判断字符串是否为空

 @param item 字符串
 @return 是否为空
 */
int fd_is_string_empty(char *item);

/**
 检查CPU的字节序

 @return 1，高字节序 2，低字节序
 */
int fd_cpu_byteorder(void);

/**
 自动调整字节序
 调整字节序,默认传入的字节序为低字节序,如果系统为高字节序,转化为高字节序
 c语言字节序的写入是低字节序的,读取默认也是低字节序的
 java语言字节序默认是高字节序的
 
 @param item char数组
 */
void fd_adjust_byteorder(char item[4]);


void ReverseArray(char arr[], long size);

#endif
