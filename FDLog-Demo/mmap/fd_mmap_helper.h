#ifndef FD_MMAP_MMAP
#define FD_MMAP_MMAP 1
#endif

#ifndef FD_MMAP_MEMORY
#define FD_MMAP_MEMORY 0
#endif

#ifndef FD_MMAP_FAIL
#define FD_MMAP_FAIL -1
#endif

#ifndef FD_MMAP_LENGTH
#define FD_MMAP_LENGTH 150 * 1024 //150k
#endif

#ifndef FD_MEMORY_LENGTH
#define FD_MEMORY_LENGTH 150 * 1024 //150k
#endif

#ifndef fd_mmap_helper_h
#define fd_mmap_helper_h

#include "fd_core_model.h"


/**
 开启MMAP

 @param _filepath MMAP 缓存文件地址
 @param buffer MMAP绑定文件的内存指针 绑定成功，操作内存 等于操作文件。(当MMAP无法malloc创建，buffer指向 内存缓存指针)
 @return 开启状态 (1)MMAP (0)MEMORY (-1)FAIL
 */
int fd_open_mmap_file1(FDLOGMODEL *model,char *_filepath, unsigned char **buffer);


#endif
