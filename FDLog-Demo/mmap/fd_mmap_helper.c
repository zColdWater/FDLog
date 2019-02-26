#include "fd_mmap_helper.h"
#include "fd_console_helper.h"

int fd_open_mmap_file(char *_filepath, unsigned char **buffer, unsigned char **cache) {
    int back = FD_MMAP_FAIL;
    
    // 如果MMAP缓存文件不存在 使用内存缓存
    if (NULL == _filepath || 0 == strnlen(_filepath, 128)) {
        back = FD_MMAP_MEMORY;
    } else { // MMAP缓存文件存在
        unsigned char *p_map = NULL;
        int size = FD_MMAP_LENGTH;
        // 打开文件流
        int fd = open(_filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); //后两个添加权限
        
        // 是否需要检查mmap缓存文件重新检查
        int isNeedCheck = 0;
        
        // fd：若所有欲核查的权限都通过了检查则返回0 值, 表示成功, 只要有一个权限被禁止则返回-1.
        if (fd != -1) { //保护
            int isFileOk = 0;
            // 先判断文件是否有值，再mmap内存映射
            FILE *file = fopen(_filepath, "rb+");
            if (NULL != file) {
                
                // 移动指针到文件最后
                fseek(file, 0, SEEK_END);
                // 获取整个文件大小
                long longBytes = ftell(file);
                
                // MMAP缓存文件长度 小于 FD_MMAP_LENGTH
                if (longBytes < FD_MMAP_LENGTH) {
                    
                    // 移动指针到文件开头
                    fseek(file, 0, SEEK_SET);
                    
                    // 创建FD_MMAP_LENGTH长度 字符串数组
                    char zero_data[size];
                    // 清空内存中的字节
                    memset(zero_data, 0, size);
                    
                    size_t _size = 0;
                    // 将文件内容全部置0
                    _size = fwrite(zero_data, sizeof(char), size, file);
                    // 清空缓存流
                    fflush(file);
                    
                    // 如果写入的字节 等于 MMAP文件设置的字节，证明全部写入。
                    if (_size == size) {
                        fd_printf("copy data 2 mmap file success\n");
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

            // 当MMAP文件全部清空置0 并且标记MMAP缓存文件可用 加强保护，对映射的文件要有一个适合长度的文件
            if (isNeedCheck) {
                FILE *file = fopen(_filepath, "rb");
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
            
            // 如果MMAP缓存文件状态正确
            if (isFileOk) {
                // 使用MMAP函数 绑定文件 返回操作文件指针。
                p_map = (unsigned char *) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            }
            
            if (p_map != MAP_FAILED && NULL != p_map && isFileOk) { // MMAP绑定成功
                back = FD_MMAP_MMAP;
            } else { // MMAP绑定失败
                back = FD_MMAP_MEMORY;
                fd_printf("open mmap fail , reason : %s \n", strerror(errno));
            }
            
            // 关闭文件
            close(fd);

            if (back == FD_MMAP_MMAP &&
                access(_filepath, F_OK) != -1) { //在返回mmap前,做最后一道判断，如果有mmap文件才用mmap
                back = FD_MMAP_MMAP;
                // 给外面buff 赋予 mmap文件操作指针
                *buffer = p_map;
            } else {
                back = FD_MMAP_MEMORY;
                if (NULL != p_map)
                    // 释放
                    munmap(p_map, size);
            }
        } else {
            // 权限被禁止 打印输出
            fd_printf("open(%s) fail: %s\n", _filepath, strerror(errno));
        }
    }

    // 申请内存缓存
    int size = FD_MEMORY_LENGTH;
    unsigned char *tempData = malloc(size);
    if (NULL != tempData) {
        memset(tempData, 0, size);
        
        // 给外面缓存 赋予 内存缓存
        *cache = tempData;
        if (back != FD_MMAP_MMAP) {
            // 给外面buff 赋予 内存缓存
            *buffer = tempData;
            back = FD_MMAP_MEMORY; //如果文件打开失败、如果mmap映射失败，走内存缓存
        }
    } else { // 内存缓存申请失败
        if (back != FD_MMAP_MMAP)
            back = FD_MMAP_FAIL;
    }
    return back;
}
