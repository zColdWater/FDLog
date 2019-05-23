#include "fd_mmap_helper.h"
#include "fd_console_helper.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


int fd_open_mmap_file(FDLOGMODEL **model,char *mmap_file_path, unsigned char **buffer) {
    
    FDLOGMODEL* model1 = *model;
    
    if (model1 == NULL) {
        fd_printf("FDLog: FDLog fd_open_mmap_file: FDLOGMODEL model is NULL ");
        return 0;
    }
    
    int back = FD_MMAP_FAIL;
    model1->is_bind_mmap = 0;
    
    char filepath[1024] = {'0'};
    strcpy(filepath, mmap_file_path);
    
    if ( !(NULL == mmap_file_path || 0 == strnlen(mmap_file_path, 128)) ) {

        unsigned char *p_map = NULL;
        int size = FD_MMAP_LENGTH;
        int fd = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); //后两个添加权限
        int isNeedCheck = 0;
        
        if (fd != -1) { //保护
            int isFileOk = 0;
            FILE *file = fopen(filepath, "rb+");
            if (NULL != file) {
                fseek(file, 0, SEEK_END);
                long longBytes = ftell(file);
                if (longBytes < FD_MMAP_LENGTH) {
                    fseek(file, 0, SEEK_SET);
                    char zero_data[size];
                    memset(zero_data, 0, size);
                    size_t _size = 0;
                    _size = fwrite(zero_data, sizeof(char), size, file);
                    fflush(file);
                    if (_size == size) {
                        fd_printf("FDLog: copy data 2 mmap file success\n");
                        isFileOk = 1;
                        isNeedCheck = 1;
                    } else { // 写入存在失败
                        isFileOk = 0;
                    }
                } else {
                    isFileOk = 1;
                }
                fclose(file);
            } else { // 文件创建失败 MMAP缓存文件不可用
                isFileOk = 0;
            }
            
            if (isNeedCheck) {
                FILE *file = fopen(filepath, "rb");
                if (file != NULL) {
                    fseek(file, 0, SEEK_END);
                    long longBytes = ftell(file);
                    if (longBytes >= FD_MMAP_LENGTH) {
                        isFileOk = 1;
                    } else {
                        isFileOk = 0;
                    }
                    fclose(file);
                } else {
                    isFileOk = 0;
                }
            }
            
            if (isFileOk) {
                p_map = (unsigned char *) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            }
            
            if (p_map != MAP_FAILED && NULL != p_map && isFileOk) { // MMAP绑定成功
                back = FD_MMAP_MMAP;
                model1->is_bind_mmap = 1;
            } else { // MMAP绑定失败
                fd_printf("FDLog: open mmap fail , reason : %s \n", strerror(errno));
            }
            close(fd);
            if (back == FD_MMAP_MMAP && access(filepath, F_OK) != -1) {
                back = FD_MMAP_MMAP;
                *buffer = p_map;
                model1->is_bind_mmap = 1;
            } else {
                if (NULL != p_map)
                    munmap(p_map, size);
            }
        } else {
            fd_printf("FDLog: open(%s) fail: %s\n", filepath, strerror(errno));
        }
    }
    
    *model = model1;
    return back;
}
