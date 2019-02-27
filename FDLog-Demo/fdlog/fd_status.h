#ifndef fd_status_h
#define fd_status_h

#define FD_INIT_STATUS "clogan_init" //初始化函数
#define FD_INIT_SUCCESS_MMAP -1010 //初始化成功, mmap内存
#define FD_INIT_SUCCESS_MEMORY -1020 //初始化成功, 堆内存
#define FD_INIT_FAIL_NOCACHE -1030 //初始化失败 , 没有缓存
#define FD_INIT_FAIL_NOMALLOC  -1040 //初始化失败 , 没有堆内存
#define FD_INIT_FAIL_HEADER  -1050 //初始化失败 , 初始化头失败
#define FD_INIT_INITPUBLICKEY_ERROR  -1061//初始化失败，初始化公钥失败
#define FD_INIT_INITBLICKENCRYPT_ERROR -1062//初始化失败，公钥加密失败
#define FD_INIT_INITBASE64_ERROR -1063 //初始化失败，base64失败
#define FD_INIT_INITMALLOC_ERROR -1064 //初始化失败 malloc分配失败

#define FD_OPEN_STATUS "clogan_open" //打开文件函数
#define FD_OPEN_SUCCESS -2010 //打开文件成功
#define FD_OPEN_FAIL_IO -2020 //打开文件IO失败
#define FD_OPEN_FAIL_ZLIB -2030 //打开文件zlib失败
#define FD_OPEN_FAIL_MALLOC -2040 //打开文件malloc失败
#define FD_OPEN_FAIL_NOINIT -2050 //打开文件没有初始化失败
#define FD_OPEN_FAIL_HEADER -2060 //打开文件头失败

#define FD_WRITE_STATUS "clogan_write" //写入函数
#define FD_WRITE_SUCCESS -4010 //写入日志成功
#define FD_WRITE_FAIL_PARAM -4020 //写入失败, 可变参数错误
#define FD_WRITE_FAIL_MAXFILE -4030 //写入失败,超过文件最大值
#define FD_WRITE_FAIL_MALLOC -4040 //写入失败,malloc失败
#define FD_WRITE_FAIL_HEADER -4050 //写入头失败

#define FD_FLUSH_STATUS "clogan_flush" //强制写入函数
#define FD_FLUSH_SUCCESS -5010 //fush日志成功
#define FD_FLUSH_FAIL_INIT -5020 //初始化失败,日志flush不成功

#endif
