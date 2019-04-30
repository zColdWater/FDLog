#ifndef FD_MMAP_MMAP
#define FD_MMAP_MMAP 1
#endif

#ifndef FD_MMAP_FAIL
#define FD_MMAP_FAIL 0
#endif

#ifndef FD_MMAP_LENGTH
#define FD_MMAP_LENGTH 150 * 1024 //150k MMAP缓存文件的大小
#endif

#ifndef fd_mmap_helper_h
#define fd_mmap_helper_h

#include "fd_core_model.h"



/*
 * Function: fd_open_mmap_file
 * ----------------------------
 *   Returns weather bind mmap file the result of success or failture
 *
 *   model: log model
 *   mmap_file_path: need bind of file path
 *   buffer: bind mmap file point
 *
 *   returns: the int value 0 is failture 1 is success.
 */
int fd_open_mmap_file(FDLOGMODEL **model,char *mmap_file_path, unsigned char **buffer);


#endif
