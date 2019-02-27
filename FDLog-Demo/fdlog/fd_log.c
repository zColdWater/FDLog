//
//  fd_log.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_log.h"
#include "fd_mmap_helper.h"
#include "fd_construct_data.h"
#include "cJSON.h"
#include "fd_json_helper.h"
#include "fd_zlib_helper.h"
#include "fd_aes_helper.h"
#include "fd_directory_helper.h"
#include "fd_base_helper.h"
#include "fd_console_helper.h"
#include "fd_status.h"
#include <string.h>



static int is_init_ok = 0; // fdlog_init 是否成功
static int is_open_ok = 0; // fdlog_open 是否成功

static unsigned char *_logan_buffer = NULL; //缓存Buffer (不释放)

static char *_dir_path = NULL; //目录路径 (不释放)

static char *_mmap_file_path = NULL; //mmap文件路径 (不释放)

static int buffer_length = 0; //缓存区域的大小

static unsigned char *_cache_buffer_buffer = NULL; //临时缓存文件 (不释放)

static int buffer_type; //缓存区块的类型 MMAP文件 或者 内存缓存

static long max_file_len = FD_LOGFILE_MAXLENGTH; //文件最大长度

static fd_logmodel *fdlog_model = NULL; //(不释放) 核心日志模型


//更新总数据和最后的count的数据到内存中
void fd_update_length(fd_logmodel *model) {
    unsigned char *temp = NULL;
    if (NULL != model->total_point) {
        temp = model->total_point;
        *temp = model->total_len;
        temp++;
        *temp = model->total_len >> 8;
        temp++;
        *temp = model->total_len >> 16;
    }
    
    if (NULL != model->content_lent_point) {
        temp = model->content_lent_point;
        // 为了兼容java,采用高字节序
        *temp = model->content_len >> 24;
        temp++;
        *temp = model->content_len >> 16;
        temp++;
        *temp = model->content_len >> 8;
        temp++;
        *temp = model->content_len;
    }
}


void fd_init_encrypt_key(fd_logmodel *logan_model) {
    fd_aes_inflate_iv(logan_model->aes_iv);
}

/**
 * 确立最后的长度指针位置和最后的写入指针位置
 */
void fd_restore_last_position(fd_logmodel *model) {
    
    // 声明临时指针 指向 last_point
    unsigned char *temp = model->last_point;
    
    // 给首地址赋值 \01
    *temp = FD_WRITE_PROTOCOL_HEADER;
    
    // 总长度 +1
    // 指针向后移动1
    model->total_len++;
    temp++;
    
    model->content_lent_point = temp; // 内容的指针
    
    
    *temp = model->content_len >> 24;
    
    // 总长度 +1
    // 指针向后移动1
    model->total_len++;
    temp++;
    
    *temp = model->content_len >> 16;
    
    // 总长度 +1
    // 指针向后移动1
    model->total_len++;
    temp++;
    
    
    *temp = model->content_len >> 8;
    
    // 总长度 +1
    // 指针向后移动1
    model->total_len++;
    temp++;
    
    
    *temp = model->content_len;
    
    // 总长度 +1
    // 指针向后移动1
    model->total_len++;
    temp++;
    
    
    model->last_point = temp;
    
    fd_printf("restore_last_position_fd > content_len : %d\n", model->content_len);
}

// 对空的文件插入一行头文件做标示 日志文件 完整的日志记录过程
void fd_insert_header_file(fd_logmodel *loganModel) {
    char *log = "fd header";
    int flag = 1; // 头部类型
    long long local_time = fd_get_system_current(); // 当前时间
    char *thread_name = "fd"; // 线程名字
    long long thread_id = 1; // 线程 id
    int ismain = 1; // 主线程
    FD_Construct_Data *data = fd_construct_json_data(log, flag, local_time, thread_name,
                                                             thread_id, ismain); // 日志信息模型
    if (NULL == data) {
        return;
    }
    fd_logmodel temp_model; //临时的fd_model
    int status_header = 1; // 表明 状态是 文件头日志
    memset(&temp_model, 0, sizeof(fd_logmodel)); // 清空结构体内存数据
    if (true != fd_init_zlib(&temp_model)) { //
        status_header = 0;
    }
    
    if (status_header) {
        // 对数据加密
        fd_init_encrypt_key(&temp_model);
        
        // 数据长度
        int length = data->data_len * 10; // 为啥 * 10 ？
        
        // 临时内存数组
        unsigned char temp_memory[length];
        
        // 置空
        memset(temp_memory, 0, length);
        
        // 设置其长度
        temp_model.total_len = 0;
        
        // 最后的指针
        temp_model.last_point = temp_memory;
        fd_restore_last_position(&temp_model); // 确立最后的长度指针位置和最后的写入指针位置
        
        // 压缩数据
        fd_zlib_compress(&temp_model, data->data, data->data_len);
        
        // 结束压缩 对剩余数据 进行填充
        fd_zlib_end_compress(&temp_model);
        
        // 更新内容的长度
        fd_update_length(&temp_model);
        
        fwrite(temp_memory, sizeof(char), temp_model.total_len, loganModel->file);//写入到文件中
        fflush(fdlog_model->file);
        loganModel->file_len += temp_model.total_len; //修改文件大小
    }
    
    if (temp_model.is_malloc_zlib) {
        free(temp_model.strm);
        temp_model.is_malloc_zlib = 0;
    }
    fd_construct_data_delete(data);
}


/**
 初始化文件相关
 
 @param model 日志模型
 @return int
 */
int fd_init_file(fd_logmodel *model) {
    int is_ok = 0;
    if (FD_FILE_OPEN == model->file_stream_type) {
        return 1;
    } else {
        FILE *file_temp = fopen(model->file_path, "ab+");
        if (NULL != file_temp) {  //初始化文件流开启
            model->file = file_temp;
            fseek(file_temp, 0, SEEK_END);
            long longBytes = ftell(file_temp);
            model->file_len = longBytes;
            model->file_stream_type = FD_FILE_OPEN;
            is_ok = 1;
        } else {
            model->file_stream_type = FD_FILE_NONE;
        }
    }
    return is_ok;
}


//文件写入磁盘、更新文件大小
void fd_write_dest(void *point, size_t size, size_t length, fd_logmodel *loganModel) {
    if (!fd_is_file_exist(loganModel->file_path)) { //如果文件被删除,再创建一个文件
        if (fdlog_model->file_stream_type == FD_FILE_OPEN) {
            fclose(fdlog_model->file);
            fdlog_model->file_stream_type = FD_FILE_CLOSE;
        }
        if (NULL != _dir_path) {
            if (!fd_is_file_exist(_dir_path)) {
                fd_makedir(_dir_path);
            }
            fd_init_file(fdlog_model);
            fd_printf("fd_write > create log file , restore open file stream \n");
        }
    }
    if (FD_EMPTY_FILE == loganModel->file_len) { // 本地文件日志 如果是空文件插入一行fd的头文件
        fd_insert_header_file(loganModel);
    }
    fwrite(point, sizeof(char), fdlog_model->total_len, fdlog_model->file);// 写入到文件中 logan_model->total_point
    //清空缓冲区
    fflush(fdlog_model->file);
    loganModel->file_len += loganModel->total_len; //修改文件大小
}


//对fd_model数据做还原
void fd_clear(fd_logmodel *logan_model) {
    logan_model->total_len = 0;
    
    if (logan_model->zlib_type == FD_ZLIB_END) { //因为只有ZLIB_END才会释放掉内存,才能再次初始化
        memset(logan_model->strm, 0, sizeof(z_stream));
        logan_model->zlib_type = FD_ZLIB_NONE;
        fd_init_zlib(logan_model);
    }
    logan_model->remain_data_len = 0;
    logan_model->content_len = 0;
    logan_model->last_point = logan_model->total_point + FD_MMAP_TOTALLEN;
    fd_restore_last_position(logan_model);
    fd_init_encrypt_key(logan_model);
    logan_model->total_len = 0;
    fd_update_length(logan_model);
    logan_model->total_len = FD_WRITEPROTOCOL_HEAER_LENGTH;
}


void fd_write_flush() {
    if (fdlog_model->zlib_type == FD_ZLIB_ING) {
        fd_zlib_end_compress(fdlog_model);
        fd_update_length(fdlog_model);
    }
    if (fdlog_model->total_len > FD_WRITEPROTOCOL_HEAER_LENGTH) {
        unsigned char *point = fdlog_model->total_point;
        point += FD_MMAP_TOTALLEN; // 移动 MMAP 3位
        fd_write_dest(point, sizeof(char), fdlog_model->total_len, fdlog_model); // 写入磁盘
        fd_printf("write_flush_fd > logan total len : %d \n", fdlog_model->total_len);
        fd_clear(fdlog_model); // 清空logan_model对象
    }
}



/**
 强制写入文件。建议在崩溃或者退出程序的时候调用

 @return 状态
 */
int fdlog_flush(void) {
    int back = FD_FLUSH_FAIL_INIT;
    if (!is_init_ok || NULL == fdlog_model) {
        return back;
    }
    fd_write_flush();
    back = FD_FLUSH_SUCCESS;
    fd_printf(" fd_flush > write flush\n");
    return back;
}


/**
 写入MMAP缓存文件

 @param path MMAP缓存文件地址
 @param temp 内容
 */
void fd_write_mmap_data(char *path, unsigned char *temp) {
    fdlog_model->total_point = temp;
    fdlog_model->file_path = path; // 日志文件的路径
    char len_array[] = {'\0', '\0', '\0', '\0'};
    len_array[0] = *temp;
    temp++;
    len_array[1] = *temp;
    temp++;
    len_array[2] = *temp;
    
    fd_adjust_byteorder(len_array);//调整字节序,默认为低字节序,在读取的地方处理
    
    int *total_len = (int *) len_array;
    
    int t = *total_len;
    fd_printf("write_mmapdata_fd > buffer total length %d\n", t);
    if (t > FD_WRITEPROTOCOL_HEAER_LENGTH && t < FD_MMAP_LENGTH) {
        fdlog_model->total_len = t;
        if (NULL != fdlog_model) {
            if (fd_init_file(fdlog_model)) {
                fdlog_model->is_ok = 1;
                fdlog_model->zlib_type = FD_ZLIB_NONE;
                fdlog_flush();
                fclose(fdlog_model->file);
                fdlog_model->file_stream_type = FD_FILE_CLOSE;
                
            }
        }
    } else {
        fdlog_model->file_stream_type = FD_FILE_NONE;
    }
    fdlog_model->total_len = 0;
    fdlog_model->file_path = NULL;
}


/**
 读取MMAP缓存文件内容
 
 @param path_dirs MMAP缓存文件路径
 */
void fd_read_mmap_data(const char *path_dirs) {
    if (buffer_type == FD_MMAP_MMAP) {
        unsigned char *temp = _logan_buffer;
        unsigned char *temp2 = NULL;
        char i = *temp;
        if (FD_MMAP_HEADER_PROTOCOL == i) { //没有读到MMAP协议头 直接跳过
            temp++;
            char len_array[] = {'\0', '\0', '\0', '\0'};
            len_array[0] = *temp;
            temp++;
            len_array[1] = *temp;
            fd_adjust_byteorder(len_array);
            int *len_p = (int *) len_array;
            temp++;
            temp2 = temp;
            int len = *len_p;
            
            fd_printf("read_mmapdata_fd > path's json length : %d\n", len);
            
            if (len > 0 && len < 1024) {
                temp += len;
                i = *temp;
                if (FD_MMAP_TAIL_PROTOCOL == i) {
                    char dir_json[len];
                    memset(dir_json, 0, len);
                    memcpy(dir_json, temp2, len);
                    fd_printf("dir_json %s\n", dir_json);
                    cJSON *cjson = cJSON_Parse(dir_json);
                    
                    if (NULL != cjson) {
                        cJSON *dir_str = cJSON_GetObjectItem(cjson,
                                                             FD_VERSION_KEY);  //删除json根元素释放
                        cJSON *path_str = cJSON_GetObjectItem(cjson, FD_PATH_KEY);
                        if ((NULL != dir_str && cJSON_Number == dir_str->type &&
                             FD_VERSION_NUMBER == dir_str->valuedouble) &&
                            (NULL != path_str && path_str->type == cJSON_String &&
                             !fd_is_string_empty(path_str->valuestring))) {
                                
                                fd_printf(
                                          "read_mmapdata_fd > dir , path and version : %s || %s || %lf\n",
                                          path_dirs, path_str->valuestring, dir_str->valuedouble);
                                
                                size_t dir_len = strlen(path_dirs);
                                size_t path_len = strlen(path_str->valuestring);
                                size_t length = dir_len + path_len + 1;
                                char file_path[length];
                                memset(file_path, 0, length);
                                memcpy(file_path, path_dirs, dir_len);
                                strcat(file_path, path_str->valuestring);
                                temp++;
                                fd_write_mmap_data(file_path, temp);
                            }
                        cJSON_Delete(cjson);
                    }
                }
            }
        }
    }
}










int fdlog_init(const char *cache_dirs,
               const char *path_dirs,
               int max_file,
               const char *encrypt_key16,
               const char *encrypt_iv16) {
    
    int back = FD_INIT_FAIL_HEADER;
    if (is_init_ok ||
        NULL == cache_dirs || strnlen(cache_dirs, 11) == 0 ||
        NULL == path_dirs || strnlen(path_dirs, 11) == 0 ||
        NULL == encrypt_key16 || NULL == encrypt_iv16) {
        // 缺少必要参数，初始化失败。
        back = FD_INIT_FAIL_HEADER;
        return back;
    }
    
    
    // 如果设置了 max_file 大小 使用传入值，否则使用默认大小。
    if (max_file > 0) {
        max_file_len = max_file;
    } else {
        max_file_len = FD_LOGFILE_MAXLENGTH;
    }
    
    // 初始化时 , _dir_path和_mmap_file_path是非空值,先释放,再NULL
    if (NULL != _dir_path) {
        free(_dir_path);
        _dir_path = NULL;
    }
    if (NULL != _mmap_file_path) {
        free(_mmap_file_path);
        _mmap_file_path = NULL;
    }
    
    // 初始化AES加密 传入 “Key” 和 “向量”
    fd_aes_init_key_iv(encrypt_key16, encrypt_iv16);
    
    size_t path1 = strlen(cache_dirs); // 缓存文件夹的路径
    size_t path2 = strlen(FD_CACHE_DIR); // 缓存文件夹的名字
    size_t path3 = strlen(FD_CACHE_FILE); // 缓存文件的名字
    size_t path4 = strlen(FD_DIVIDE_SYMBOL); // 分割符号
    
    int isAddDivede = 0;
    char d = *(cache_dirs + path1 - 1); // 判断 cache_dirs 最后一个字符 不是 / 需要加分隔符
    if (d != '/') {
        isAddDivede = 1;
    }
    
    // 计算完整路径长度，方便后面申请堆内存
    size_t total = path1 + (isAddDivede ? path4 : 0) + path2 + path4 + path3 + 1;
    char *cache_path = malloc(total);
    if (NULL != cache_path) {
        _mmap_file_path = cache_path; //保持mmap文件路径,如果初始化失败,注意释放_mmap_file_path
    } else {
        is_init_ok = 0;
        fd_printf("fd_init > malloc memory fail for mmap_file_path \n");
        back = FD_INIT_FAIL_NOMALLOC;
        return back;
    }
    
    memset(cache_path, 0, total); // 清空申请的堆内存
    strcpy(cache_path, cache_dirs); // 将参数cache_dirs赋给cache_path
    if (isAddDivede)
        strcat(cache_path, FD_DIVIDE_SYMBOL);
    
    strcat(cache_path, FD_CACHE_DIR);
    strcat(cache_path, FD_DIVIDE_SYMBOL);
    
    fd_makedir(cache_path); //创建保存mmap文件的目录
    strcat(cache_path, FD_CACHE_FILE);
    
    size_t dirLength = strlen(path_dirs);
    
    isAddDivede = 0;
    d = *(path_dirs + dirLength - 1);
    if (d != '/') {
        isAddDivede = 1;
    }
    total = dirLength + (isAddDivede ? path4 : 0) + 1;
    
    char *dirs = (char *) malloc(total); //缓存文件目录
    
    if (NULL != dirs) {
        _dir_path = dirs; //日志写入的文件目录
    } else {
        is_init_ok = 0;
        fd_printf("fd_init > malloc memory fail for _dir_path \n");
        back = FD_INIT_FAIL_NOMALLOC;
        return back;
    }
    memset(dirs, 0, total);
    memcpy(dirs, path_dirs, dirLength);
    if (isAddDivede)
        strcat(dirs, FD_DIVIDE_SYMBOL);
    fd_makedir(_dir_path); //创建缓存目录,如果初始化失败,注意释放_dir_path
    
    
    // _logan_buffer _cache_buffer_buffer 赋值
    int flag = FD_MMAP_FAIL;
    if (NULL == _logan_buffer) { // 由于 _logan_buffer 指针 绑定了 MMAP
        if (NULL == _cache_buffer_buffer) { // 由于 _cache_buffer_buffer 指针 没有绑定 MMAP
            flag = fd_open_mmap_file(cache_path, &_logan_buffer, &_cache_buffer_buffer);
        } else {
            flag = FD_MMAP_MEMORY;
        }
    } else {
        flag = FD_MMAP_MMAP;
    }
    
    
    // 判断缓存是哪种类型，然后改变状态。
    if (flag == FD_MMAP_MMAP) { // 使用MMAP 内存 文件绑定
        buffer_length = FD_MMAP_LENGTH;
        buffer_type = FD_MMAP_MMAP;
        is_init_ok = 1;
        back = FD_INIT_SUCCESS_MMAP;
    } else if (flag == FD_MMAP_MEMORY) { // 使用内存
        buffer_length = FD_MEMORY_LENGTH;
        buffer_type = FD_MMAP_MEMORY;
        is_init_ok = 1;
        back = FD_INIT_SUCCESS_MEMORY;
    } else if (flag == FD_MMAP_FAIL) { // 初始化失败
        is_init_ok = 0;
        back = FD_INIT_FAIL_NOCACHE;
    }
    
    
    
    if (is_init_ok) { // 初始化成功
        
        // 如果 fdlog_model 静态变量不存在，重新创建一份新的。
        if (NULL == fdlog_model) {
            fdlog_model = malloc(sizeof(fd_logmodel)); // 申请结构体字节总数
            if (NULL != fdlog_model) { //堆非空判断 , 如果为null , 就失败
                memset(fdlog_model, 0, sizeof(fd_logmodel));
            } else {
                is_init_ok = 0;
                fd_printf("fd_init > malloc memory fail for logan_model\n");
                back = FD_INIT_FAIL_NOMALLOC;
                return back;
            }
        }
        
        // 如果读到了MMAP缓存文件存在 日志内容，那么将文件存入到本地日志内容中。
        // 当没有读取到MMAP中的日志内容，那么直接退出。
        if (flag == FD_MMAP_MMAP) { //MMAP的缓存模式,从缓存的MMAP中读取数据
            fd_read_mmap_data(_dir_path);
        }
        
        
        fd_printf("fd_init > logan init success\n");
        
        
    } else { // 初始化失败
        
        fd_printf("fd_open > logan init fail\n");
        
        // 释放 _dir_path
        if (NULL == _dir_path) {
            free(_dir_path);
            _dir_path = NULL;
        }
        
        // 释放 _mmap_file_path
        if (NULL == _mmap_file_path) {
            free(_mmap_file_path);
            _mmap_file_path = NULL;
        }
        
    }
    return back;
}



/*
 * 对mmap添加header和确定总长度位置
 */
void fd_add_mmap_header(char *content, fd_logmodel *model) {
    size_t content_len = strlen(content) + 1;
    size_t total_len = content_len;
    char *temp = (char *) model->buffer_point;
    *temp = FD_MMAP_HEADER_PROTOCOL;
    temp++;
    
    *temp = total_len;
    temp++;
    
    *temp = total_len >> 8;
    temp++;
    
    fd_printf("\n add_mmap_header_fd len %d\n", total_len);
    
    memcpy(temp, content, content_len);
    temp += content_len;
    
    *temp = FD_MMAP_TAIL_PROTOCOL;
    temp++;
    
    model->total_point = (unsigned char *) temp; // 总数据的total_length的指针位置
    model->total_len = 0; // 头部不算
}



int fdlog_open(const char *pathname) {
    
    int back = FD_OPEN_FAIL_NOINIT;
    
    // 必须初始化成功，不成功无法继续 fd_open 方法。
    if (!is_init_ok) {
        back = FD_OPEN_FAIL_NOINIT;
        return back;
    }
    
    // 必要参数不能缺失，如果不满足条件 状态设置为 OPEN 失败 退出。
    is_open_ok = 0;
    if (NULL == pathname || 0 == strnlen(pathname, 128) || NULL == _logan_buffer ||
        NULL == _dir_path ||
        0 == strnlen(_dir_path, 128)) {
        back = FD_OPEN_FAIL_HEADER;
        return back;
    }
    
    // 当total_len大于写
    if (NULL != fdlog_model) { //回写到日志中
        
        if (fdlog_model->total_len > FD_WRITEPROTOCOL_HEAER_LENGTH) {
            fdlog_flush(); // 直接强行写入 本地日志文件
        }
        
        if (fdlog_model->file_stream_type == FD_FILE_OPEN) {
            fclose(fdlog_model->file);
            fdlog_model->file_stream_type = FD_FILE_CLOSE;
        }
        if (NULL != fdlog_model->file_path) {
            free(fdlog_model->file_path);
            fdlog_model->file_path = NULL;
        }
        fdlog_model->total_len = 0;
    } else {
        fdlog_model = malloc(sizeof(fd_logmodel));
        if (NULL != fdlog_model) {
            memset(fdlog_model, 0, sizeof(fd_logmodel));
        } else {
            fdlog_model = NULL; //初始化Logan_model失败,直接退出
            is_open_ok = 0;
            back = FD_OPEN_FAIL_MALLOC;
            return back;
        }
    }
    char *temp = NULL;
    // 拼接出日志文件PATH
    size_t file_path_len = strlen(_dir_path) + strlen(pathname) + 1;
    char *temp_file = malloc(file_path_len); // 日志文件路径
    if (NULL != temp_file) {
        memset(temp_file, 0, file_path_len);
        temp = temp_file;
        memcpy(temp, _dir_path, strlen(_dir_path));
        temp += strlen(_dir_path);
        memcpy(temp, pathname, strlen(pathname)); //创建文件路径
        fdlog_model->file_path = temp_file;
        
        if (!fd_init_file(fdlog_model)) {  //初始化文件IO和文件大小 然后给logan_model属性赋值
            is_open_ok = 0;
            back = FD_OPEN_FAIL_IO;
            return back;
        }
        
        if (fd_init_zlib(fdlog_model) != true) { //初始化zlib压缩
            is_open_ok = 0;
            back = FD_OPEN_FAIL_ZLIB;
            return back;
        }
        
        fdlog_model->buffer_point = _logan_buffer;
        
        if (buffer_type == FD_MMAP_MMAP) {  //如果是MMAP,缓存文件目录和文件名称
            cJSON *root = NULL;
            fd_json_map *map = NULL;
            root = cJSON_CreateObject();
            map = fd_create_json_map();
            char *back_data = NULL;
            if (NULL != root) {
                if (NULL != map) {
                    fd_add_item_number(map, FD_VERSION_KEY, FD_VERSION_NUMBER);
                    fd_add_item_string(map, FD_PATH_KEY, pathname);
                    fd_inflate_json_by_map(root, map);
                    back_data = cJSON_PrintUnformatted(root);
                }
                cJSON_Delete(root);
                if (NULL != back_data) {
                    fd_add_mmap_header(back_data, fdlog_model); //确定total_point指向
                    free(back_data);
                } else {
                    fdlog_model->total_point = _logan_buffer;
                    fdlog_model->total_len = 0;
                }
            } else {
                fdlog_model->total_point = _logan_buffer;
                fdlog_model->total_len = 0;
            }
            
            fdlog_model->last_point = fdlog_model->total_point + FD_MMAP_TOTALLEN;
            
            if (NULL != map) {
                fd_delete_json_map(map);
            }
        } else {
            fdlog_model->total_point = _logan_buffer;
            fdlog_model->total_len = 0;
            fdlog_model->last_point = fdlog_model->total_point + FD_MMAP_TOTALLEN; // last_point 赋值
        }
        
        fd_restore_last_position(fdlog_model); // 重新确定指针的最后指向
        fd_init_encrypt_key(fdlog_model);
        fdlog_model->is_ok = 1;
        is_open_ok = 1;
    } else {
        is_open_ok = 0;
        back = FD_OPEN_FAIL_MALLOC;
        fd_printf("fd_open > malloc memory fail\n");
    }
    
    if (is_open_ok) {
        back = FD_OPEN_SUCCESS;
        fd_printf("fd_open > logan open success\n");
    } else {
        fd_printf("fd_open > logan open fail\n");
    }
    return back;
}


void fd_write2(char *data, int length) {
    if (NULL != fdlog_model && fdlog_model->is_ok) {
        fd_zlib_compress(fdlog_model, data, length); // 压缩后 加密后 写入内存
        fd_update_length(fdlog_model); //有数据操作,要更新数据长度到缓存中
        int is_gzip_end = 0; // 是否压缩结束的标志符
        
        if (!fdlog_model->file_len ||
            fdlog_model->content_len >= FD_MAX_GZIP_UTIL) { //是否一个压缩单元结束
            fd_zlib_end_compress(fdlog_model);
            is_gzip_end = 1;
            fd_update_length(fdlog_model);
        }
        printf("fd_write2 logan_model->content_len: %d \n",fdlog_model->content_len);
        
        
        int isWrite = 0;
        if (!fdlog_model->file_len && is_gzip_end) { //如果是个空文件、第一条日志写入
            isWrite = 1;
            printf("fd_write2 > 写入空文件 \n");
        } else if (buffer_type == FD_MMAP_MEMORY && is_gzip_end) { //直接写入文件
            isWrite = 1;
            printf("fd_write2 > 内存缓存 直接写入文件 \n");
        } else if (buffer_type == FD_MMAP_MMAP &&
                   fdlog_model->total_len >=
                   buffer_length / FD_WRITEPROTOCOL_DEVIDE_VALUE) { // 如果总长度 超过 buff 1/3 就写入
            isWrite = 1;
            printf("fd_write2 > MMAP缓存 总长度超过buff 1/3 写入文件 \n");
        }
        
        printf("fd_write2 > logan_model->total_len %d \n",fdlog_model->total_len);
        printf("fd_write2 > buffer_length 1/3 %d \n",buffer_length/3);
        
        
        if (isWrite) { // 写入文件
            printf("fd_write2 写入本地文件");
            fd_write_flush();
        } else if (is_gzip_end) { //如果是mmap类型,不回写IO,初始化下一步
            fdlog_model->content_len = 0;
            fdlog_model->remain_data_len = 0;
            fd_init_zlib(fdlog_model);
            fd_restore_last_position(fdlog_model);
            fd_init_encrypt_key(fdlog_model);
            printf("fd_write2 is_gzip_end 不回写IO,初始化下一步 不写入文件 \n");
        } else {
            printf("fd_write2 不写入文件 \n");
        }
    }
}



//如果数据流非常大,切割数据,分片写入
void fd_write_section(char *data, int length) {
    int size = FD_WRITE_SECTION;
    int times = length / size;
    int remain_len = length % size;
    char *temp = data;
    int i = 0;
    for (i = 0; i < times; i++) {
        fd_write2(temp, size); // 调用关键函数，日志内容 + 日志Size
        temp += size;
    }
    if (remain_len) {
        fd_write2(temp, remain_len);
    }
}


/**
 @brief 写入数据 按照顺序和类型传值(强调、强调、强调)
 @param flag 日志类型 (int)
 @param log 日志内容 (char*)
 @param local_time 日志发生的本地时间，形如1502100065601 (long long)
 @param thread_name 线程名称 (char*)
 @param thread_id 线程id (long long) 为了兼容JAVA
 @param is_main 是否为主线程，0为是主线程，1位非主线程 (int)
 */
int
fdlog_write(int flag, char *log, long long local_time, char *thread_name, long long thread_id,
             int is_main) {
    int back = FD_WRITE_FAIL_HEADER;
    if (!is_init_ok || NULL == fdlog_model || !is_open_ok) {
        back = FD_WRITE_FAIL_HEADER;
        return back;
    }
    
    if (fd_is_file_exist(fdlog_model->file_path)) {
        if (fdlog_model->file_len > 10485760) { // max_file_len
            fd_printf("fd_write > beyond max file , cant write log\n");
            back = FD_WRITE_FAIL_MAXFILE;
            return back;
        }
    } else {
        if (fdlog_model->file_stream_type == FD_FILE_OPEN) {
            fclose(fdlog_model->file);
            fdlog_model->file_stream_type = FD_FILE_CLOSE;
        }
        if (NULL != _dir_path) {
            if (!fd_is_file_exist(_dir_path)) {
                fd_makedir(_dir_path);
            }
            fd_init_file(fdlog_model);
            fd_printf("fd_write > create log file , restore open file stream \n");
        }
    }
    
    //判断MMAP文件是否存在,如果被删除,用内存缓存
    if (buffer_type == FD_MMAP_MMAP && !fd_is_file_exist(_mmap_file_path)) {
        if (NULL != _cache_buffer_buffer) {
            buffer_type = FD_MMAP_MEMORY;
            buffer_length = FD_MEMORY_LENGTH;
            
            fd_printf("fd_write > change to memory buffer");
            
            _logan_buffer = _cache_buffer_buffer;
            fdlog_model->total_point = _logan_buffer;
            fdlog_model->total_len = 0;
            fdlog_model->content_len = 0;
            fdlog_model->remain_data_len = 0;
            
            if (fdlog_model->zlib_type == FD_ZLIB_INIT) {
                fd_zlib_delete_stream(fdlog_model); //关闭已开的流
            }
            
            fdlog_model->last_point = fdlog_model->total_point + FD_MMAP_TOTALLEN;
            fd_restore_last_position(fdlog_model);
            fd_init_zlib(fdlog_model);
            fd_init_encrypt_key(fdlog_model);
            fdlog_model->is_ok = 1;
        } else {
            buffer_type = FD_MMAP_FAIL;
            is_init_ok = 0;
            is_open_ok = 0;
            _logan_buffer = NULL;
        }
    }
    
    FD_Construct_Data *data = fd_construct_json_data(log, flag, local_time, thread_name,
                                                             thread_id, is_main); // format 日志
    if (NULL != data) {
        fd_write_section(data->data, data->data_len); // 写入日志
        fd_construct_data_delete(data);
        back = FD_WRITE_SUCCESS;
    } else {
        back = FD_WRITE_FAIL_MALLOC;
    }
    return back;
}



/**
 启用Debug模式
 
 @param debug 模式
 */
void fdlog_debug(int debug) {
    fd_set_debug(debug);
}

