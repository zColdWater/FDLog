//
//  fd_log.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "fd_log.h"
#include "fd_directory_helper.h"
#include "fd_core_model.h"
#include "fd_mmap_helper.h"
#include "fd_aes_helper.h"
#include "fd_zlib_helper.h"
#include "fd_json_helper.h"

extern FDLOGMODEL *model;
extern unsigned char *mmap_ptr;
extern unsigned char *mmap_tailer_ptr;
extern int *mmap_content_len_ptr;
extern int *mmap_current_log_len_ptr;
extern int *mmap_header_content_len_ptr;
extern int *mmap_header_content_len_ptr;
extern char *mmap_header_content_ptr;
extern char *log_folder_path;
extern long *log_file_len;
extern char *mmap_cache_file_path;
extern char *log_file_path;




/// 初始化成功标志 1成功 0失败
static int is_init_ok = 0;


long getmaximum(long a[], int numberOfElements)
{
    long mymaximum = 0;
    for(int i = 0; i < numberOfElements; i++)
    {
        if(a[i] > mymaximum)
        {
            mymaximum = a[i];
        }
    }
    return mymaximum;
}


/**
 获取当前时间
 @return 时间 (使用完需要释放)
 */
char* get_current_date() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time[18];
    sprintf(time, "%d%d%d", (tm.tm_year + 1900),(tm.tm_mon + 1),(tm.tm_mday));
    char* temp = (char *)malloc(sizeof(int)*3);
    memcpy(temp, time, 18);
    return temp;
}

/**
 初始化 FDLOGMODEL
 
 @return 1成功 0失败
 */
FDLOGMODEL* init_logmodel() {
    
    FDLOGMODEL *m = (FDLOGMODEL *)malloc(sizeof(FDLOGMODEL));
    
    if (m != NULL) {
        memset(m, 0, sizeof(FDLOGMODEL));
        m->is_ready_gzip = 0;
        m->zlib_type = 0;
        m->is_bind_mmap = 0;
        memset(m->cache_remain_data, 0, 16);
        m->cache_remain_data_len = 0;
        
        m->strm = NULL;
        
        return m;
    }
    else {
        // malloc 失败 一般由于系统问题。
        return NULL;
    }
}


/**
 初始化创建文件夹
 
 @param root_path 日志根路径
 @return 1成功 0失败
 */
int init_fdlog_dirs(const char *root_path, FDLOGMODEL *m) {
    
    if ((log_folder_path == NULL) || (mmap_cache_file_path == NULL)) {
        return 0;
    }
    
    /// 缓存文件夹路径
    char cache_folder_path[FD_MAX_PATH];
    memset(cache_folder_path, 0, FD_MAX_PATH);
    
    /// 缓存文件路径
    char cache_file_path[FD_MAX_PATH];
    memset(cache_file_path, 0, FD_MAX_PATH);
    
    /// 日志文件夹路径
    char log_folder_path1[FD_MAX_PATH];
    memset(log_folder_path1, 0, FD_MAX_PATH);
    
    char path[FD_MAX_PATH];
    memset(path, 0, FD_MAX_PATH);
    strcat(path, root_path);
    strcat(path, "/");
    strcat(path, FD_LOG_FOLDER_NAME);
    
    strcpy(log_folder_path1, path);
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_FOLDER_NAME);
    
    strcpy(cache_folder_path, path);
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_NAME);
    strcpy(cache_file_path, path);
    
    int ret = fd_makedir(cache_folder_path);
    
    memcpy(mmap_cache_file_path, cache_file_path, FD_MAX_PATH);
    memcpy(log_folder_path, log_folder_path1, FD_MAX_PATH);
    
    // 失败
    if (ret < 0) {
        return 0;
    }
    else { // 成功
        return 1;
    }
}



/**
 写入mmap头部
 
 0. 【剩余数据头部】 【剩余数据】 【剩余数据尾】
 1. 【缓存文件头部】 【缓存文件头部信息】 【缓存文件尾】
 2. 【日志总长度头部】 【日志总长度】 【日志总长度尾部】
 
 @return 1成功 0失败
 */
int insert_mmap_file_header() {
    bool result = false;
    
    /// 组装MMAP文件头信息
    cJSON *root = NULL;
    fd_json_map *map = NULL;
    root = cJSON_CreateObject();
    map = fd_create_json_map();
    char *back_data = NULL;
    if (NULL != root) {
        if (NULL != map) {
            char* date = get_current_date();
            fd_add_item_number(map, FD_VERSION_KEY, FD_VERSION_NUMBER);
            fd_add_item_string(map, FD_DATE, date);
            fd_add_item_number(map, FD_SIZE, FD_MMAP_LENGTH);
            fd_inflate_json_by_map(root, map);
            back_data = cJSON_PrintUnformatted(root);
        }
        cJSON_Delete(root);
    }
    
    /// 置空缓存文件
    if (mmap_ptr != NULL) {
        memset(mmap_ptr, 0, FD_MMAP_LENGTH);
        int len = (int)strlen(back_data);
        unsigned char *temp = mmap_ptr;
        /// ==== 写入缓存文件的头部内容  ====
        *temp = FD_MMAP_FILE_HEADER;
        temp++;
        *temp = len;
        temp++;
        *temp = len >> 8;
        temp++;
        *temp = len >> 16;
        temp++;
        *temp = len >> 24;
        temp++;
        /// ==== 存入头部内容 ====
        memcpy(temp, back_data, len);
        temp += len;
        *temp = FD_MMAP_FILE_TAILER;
        temp++;
        
        
        /// ==== 写入缓存文件的总内容长度  ====
        *temp = FD_MMAP_FILE_CONTENT_HEADER;
        temp++;
        temp += sizeof(int);
        *temp = FD_MMAP_FILE_CONTENT_TAILER;
        temp ++;
        
        result = true;
        
    } else {
        printf("mmap pointer not bind memory! \n");
    }
    
    printf("back_data:%s \n",back_data);
    return result;
}



/**
 写入日志
 
 @param data 日志内容
 @return 1成功 0失败
 */
int fdlog_write_to_cache(FD_Construct_Data *data) {
    
    if (!is_init_ok) {
        printf("fdlog init failture!\n");
        return 0;
    }
    
    if (!mmap_tailer_ptr) {
        printf("mmap_tailer_ptr NULL!\n");
        return 0;
    }
    
    if (!model->is_ready_gzip) {
        printf("model->is_ready_gzip NULL! \n");
        int ret = fd_init_zlib(model);
        if (ret == 0) {
            printf("fd_init_zlib error! \n");
            return 0;
        }
    }
    
    if ((*(mmap_tailer_ptr - 1) == FD_MMAP_FILE_CONTENT_WRITE_TAILER) || (*(mmap_tailer_ptr - 1) == FD_MMAP_FILE_CONTENT_TAILER)) {
        *mmap_tailer_ptr = FD_MMAP_FILE_CONTENT_WRITE_HEADER;
        mmap_tailer_ptr += 1;
        mmap_current_log_len_ptr = (int *)mmap_tailer_ptr; // 绑定指针
        mmap_tailer_ptr += sizeof(int);
        
        int ret = fd_zlib_compress(model, data->data, data->data_len, Z_SYNC_FLUSH);
        printf("fd_zlib1 success! ret %d \n",ret);
        if (ret) {
            fd_zlib_end_compress(model);
            fd_aes_inflate_iv(model->aes_iv);
            return 1;
        }
    }
    else {
        // 日志中断 删掉之前坏数据
        printf("日志中断! \n");
    }
    
    return 0;
}


int init_global_var() {
    
    if (model == NULL) { model = init_logmodel(); }
    else {
        model->is_ready_gzip = 0;
        model->zlib_type = 0;
        model->is_bind_mmap = 0;
        model->cache_remain_data_len = 0;
        memset(model->aes_iv, 0, 16);
        memset(model->cache_remain_data, 0, 16);
        memset(model->strm, 0, sizeof(z_stream));
    }
    
    if (mmap_header_content_ptr == NULL) { mmap_header_content_ptr = (char*)calloc(1, FD_MMAP_HEADER_CONTENT_LEN); }
    else { memset(mmap_header_content_ptr, 0, FD_MMAP_HEADER_CONTENT_LEN); }
    
    if (log_folder_path == NULL) { log_folder_path = (char*)calloc(1, FD_MAX_PATH); }
    else { memset(log_folder_path, 0, FD_MAX_PATH); }
    
    if (mmap_cache_file_path == NULL) { mmap_cache_file_path = (char*)calloc(1, FD_MAX_PATH); }
    else { memset(mmap_cache_file_path,0,FD_MAX_PATH); }
    
    if (log_file_len == NULL) { log_file_len = (long *)calloc(1, sizeof(long)); }
    else { memset(log_file_len, 0, sizeof(long)); }
    
    if (log_file_path == NULL) { log_file_path = (char *)calloc(1, FD_MAX_PATH); }
    else { memset(log_file_path, 0, FD_MAX_PATH); }
    
    if ((model == NULL) ||  (log_folder_path == NULL) || (mmap_cache_file_path == NULL) || (log_file_path == NULL) || (log_file_len == NULL)) {
        return 0;
    }
    else {
        return 1;
    }
}



/**
 寻找到上一次写入的文件名
 @return 文件名
 */
char* look_for_last_logfile() {
    
    // 获取当天的日期字符串
    char* date = get_current_date();
    char* current_file_folder_name = (char *)calloc(1, FD_MAX_PATH);
    strcat(current_file_folder_name,log_folder_path);
    strcat(current_file_folder_name, "/");
    strcat(current_file_folder_name, date);
    
    int folder_exist = fd_is_file_exist(current_file_folder_name);
    if (!folder_exist) {
        free(date);
        free(current_file_folder_name);
        date = NULL;
        current_file_folder_name = NULL;
        return NULL;
    }
    
    strcat(current_file_folder_name, "/");
    
    char* files_name[1024] = {};
    int i = 0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (current_file_folder_name)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (strstr(ent->d_name, date) != NULL) {
                files_name[i] = ent->d_name;
                i++;
            }
        }
        closedir (dir);
    } else {
        perror ("");
        free(date);
        free(current_file_folder_name);
        date = NULL;
        current_file_folder_name = NULL;
        return NULL;
    }
    
    long files_name_number[i];
    for(int j=0;j<i;j++) {
        const char *c = files_name[j];
        long d = atol(c);
        files_name_number[j] = d;
        printf("%ld\n", d);
    }
    
    long max_file_name_num = getmaximum(files_name_number, i);
    if (max_file_name_num == 0) {
        free(date);
        free(current_file_folder_name);
        date = NULL;
        current_file_folder_name = NULL;
        return NULL;
    }
    
    char max_file_name[12];
    sprintf(max_file_name, "%ld", max_file_name_num);
    strcat(current_file_folder_name, max_file_name);
    
    char* maxfile_name = (char*)calloc(1, FD_MAX_PATH);
    strcpy(maxfile_name, current_file_folder_name);
    
    free(date);
    free(current_file_folder_name);
    date = NULL;
    current_file_folder_name = NULL;
    return maxfile_name;
}



/**
 创建日志文件
 
 1. 先找有没有当天文件夹
 2. 没有创建文件夹，并且进入文件夹创建日志文件。
 
 @return 1成功 0失败
 */
int create_new_logfile() {
    
    // 获取当天的日期字符串
    char* date = get_current_date();
    char* current_file_folder_name = (char *)calloc(1, FD_MAX_PATH);
    strcat(current_file_folder_name,log_folder_path);
    strcat(current_file_folder_name, "/");
    strcat(current_file_folder_name, date);
    
    int folder_exist = fd_is_file_exist(current_file_folder_name);
    if (!folder_exist) {
        int ret = fd_makedir(current_file_folder_name);
        if (ret != 0) {
            free(date);
            free(current_file_folder_name);
            date = NULL;
            current_file_folder_name = NULL;
            return 0;
        }
    }
    strcat(current_file_folder_name, "/");
    
    int same = 1;
    int additional = 1;
    char file_name[FD_MAX_PATH+sizeof(int)] = {0};
    while (same) {
        
        char additional_str[sizeof(int)];
        sprintf(additional_str, "%d", additional);
        
        char logfile[FD_MAX_PATH+sizeof(int)] = {0};
        strcat(logfile, date);
        strcat(logfile, additional_str);
        strcpy(file_name, logfile);
        printf("logfile:%s \n",logfile);
        
        /// 遍历文件夹里面的文件名
        int is_same_name = 0;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (current_file_folder_name)) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                printf ("%s\n", ent->d_name);
                if(strcmp(logfile,ent->d_name)==0) {
                    printf("日志文件有重名\n");
                    is_same_name = 1;
                    break;
                }
            }
            closedir (dir);
        } else {
            perror ("");
            free(date);
            free(current_file_folder_name);
            date = NULL;
            current_file_folder_name = NULL;
            return 0;
        }
        
        additional++;
        same = is_same_name;
    }
    strcat(current_file_folder_name, file_name);
    
    int log_file_exist = fd_is_file_exist(current_file_folder_name);
    if (!log_file_exist) {
        FILE *file_temp = fopen(current_file_folder_name, "ab+");
        if (NULL != file_temp) {  //初始化文件流开启
            fseek(file_temp, 0, SEEK_END);
            long longBytes = ftell(file_temp);
            memcpy(log_file_len, &longBytes, sizeof(long));
            if (log_file_path == NULL) {
                log_file_path = (char *)calloc(1, FD_MAX_PATH);
            } else {
                memset(log_file_path, 0, FD_MAX_PATH);
            }
            memcpy(log_file_path, current_file_folder_name, FD_MAX_PATH);
            fclose(file_temp);
        } else {
            printf("文件流打开失败!\n");
            free(date);
            free(current_file_folder_name);
            date = NULL;
            current_file_folder_name = NULL;
            return 0;
        }
    }
    
    free(date);
    free(current_file_folder_name);
    date = NULL;
    current_file_folder_name = NULL;
    return 1;
}


/**
 同步缓存文件到本地日志文件
 
 @return 1成功 0失败
 */
int fdlog_sync() {
    
    if (!is_init_ok) {
        printf("fdlog init failture!\n");
        return 0;
    }
    
    int is_new_logfile = 0;
    char* last_file_name = look_for_last_logfile();
    // 如果找不到上一次打开文件
    if (last_file_name == NULL) {
        int ret = create_new_logfile();
        if (ret) {
            is_new_logfile = 1;
        }
    }
    else {
        // 如果找到上一次打开文件 读取数据
        memset(log_file_path, 0, FD_MAX_PATH);
        memcpy(log_file_path, last_file_name, FD_MAX_PATH);
        FILE *file_temp = fopen(log_file_path, "ab+");
        if (NULL != file_temp) {  //初始化文件流开启
            fseek(file_temp, 0, SEEK_END);
            long longBytes = ftell(file_temp);
            memcpy(log_file_len, &longBytes, sizeof(long));
            fclose(file_temp);
        } else {
            printf("文件流打开失败!\n");
            free(last_file_name);
            last_file_name = NULL;
            return 0;
        }
    }
    
    if (!((log_file_len != NULL) && (mmap_tailer_ptr != NULL) && (mmap_content_len_ptr != NULL) && (*mmap_content_len_ptr > 0))) {
        return 0;
    }
    
    /// 重新创建文件绑定指针
    if ((*mmap_content_len_ptr + *log_file_len) > FD_MAX_LOG_SIZE) {
        // 如果没创建新文件 创建新文件保存日志，因为日志长度大于设定最大长度。
        if (!is_new_logfile) {
            memset(log_file_path, 0, FD_MAX_PATH);
            memset(log_file_len, 0, sizeof(long));
            create_new_logfile();
        }
    }
    
    int bind = adjust_mmap_tailer(mmap_ptr);
    printf("bind result:%d\n",bind);
    

    /// 尾巴必须指向 FD_MMAP_FILE_CONTENT_WRITE_TAILER 文件尾巴
    /// 没有结尾 先进行结尾
    if (*(mmap_tailer_ptr - 1) != FD_MMAP_FILE_CONTENT_WRITE_TAILER) {
        fd_zlib_end_compress(model);
        adjust_mmap_tailer(mmap_ptr);
        
        // 临时添加 因为多了 16字节
        mmap_tailer_ptr = mmap_tailer_ptr - 16;
        printf("*(mmap_tailer_ptr - 1): %02X \n",*(mmap_tailer_ptr - 1));
        // 加入写入结尾
        *(mmap_tailer_ptr) = FD_MMAP_FILE_CONTENT_WRITE_TAILER;
        update_mmap_content_len(1);
        adjust_mmap_tailer(mmap_ptr);
        // 重新赋值IV
        fd_aes_inflate_iv(model->aes_iv);
    }
    
    
    /// 开始写入日志到本地文件
    FILE* stream;
    stream = fopen(log_file_path, "ab+");
    printf("*(mmap_tailer_ptr - 1): %02X \n",*(mmap_tailer_ptr - 1));
    printf("*mmap_content_len_ptr: %02X \n",*mmap_content_len_ptr);

    unsigned char* temp = mmap_tailer_ptr - *mmap_content_len_ptr;
    fwrite(temp, sizeof(char), *mmap_content_len_ptr, stream);//写入到文件中
    fflush(stream);
    fclose(stream);
    *log_file_len += *mmap_content_len_ptr; //修改文件大小
    
    /// 重置缓存MMAP相关信息
    int ret = insert_mmap_file_header();
    if (ret == 0) { return 0; }
    int ret1 = adjust_mmap_tailer(mmap_ptr);
    if (ret1 == 0) { return 0; }
    
    return 1;
}



int fdlog_initialize(char* root, char* key, char* iv) {
    is_init_ok = 0;
    
    int ret = init_global_var();
    if (ret == 0) { return 0; }
    int ret1 = init_fdlog_dirs(root, model);
    if (ret1 == 0) { return 0; }
    int ret2 = fd_open_mmap_file1(model, mmap_cache_file_path, &mmap_ptr);
    if (ret2 == 0) { return 0; }
    fd_aes_init_key_iv(key, iv);
    fd_aes_inflate_iv(model->aes_iv);
    int ret3 = fd_init_zlib(model);
    if (ret3 == 0) { return 0; }
    
    // 读取缓存文件头部信息
    int ret4 = adjust_mmap_tailer(mmap_ptr);
    if (ret4) {
        cJSON* croot = cJSON_Parse(mmap_header_content_ptr);
        char* cache_date = (char*)calloc(1, 1024);
        strcpy(cache_date, cJSON_GetObjectItem(croot,FD_DATE)->valuestring);
        printf("mmap_header_content_ptr date:%s\n",cache_date);
        cJSON_Delete(croot);
        char* current_date = get_current_date();
        
        long before = atol(cache_date);
        long now = atol(current_date);
        
        if (now > before) { // 缓存文件过期，不是当天的。
            int ret1 = fdlog_sync();
            if (ret1 == 0) { return 0; }
            int ret2 = insert_mmap_file_header();
            if (ret2 == 0) { return 0; }
            int ret3 = adjust_mmap_tailer(mmap_ptr);
            if (ret3 == 0) { return 0; }
        }
        else {
            int ret0 = adjust_mmap_tailer(mmap_ptr);
            if (ret0 == 0) { return 0; }
        }
    }
    // 读取缓存文件头部信息失败
    else {
        int ret = insert_mmap_file_header();
        if (ret == 0) { return 0; }
        int ret1 = adjust_mmap_tailer(mmap_ptr);
        if (ret1 == 0) { return 0; }
    }
    
    is_init_ok = 1;
    return 1;
}



/**
 FDLog: 日志写入
 
 @param data 日志结构信息
 @return 1成功 0失败
 */
int fdlog(FD_Construct_Data *data) {
    
    if (!is_init_ok) {
        printf("fdlog init failture!\n");
        return 0;
    }
    
    /// 缓存长度已经大于设置的最大值，同步到本地文件。
    if ((float)*mmap_content_len_ptr > (float)(FD_MMAP_LENGTH * FD_MAX_MMAP_SCALE)) {
        printf("cache length more large than max!\n");
        int ret = fdlog_sync();
        if (!ret) {
            printf("sync cache to local failture!\n");
            return 0;
        }
        int ret1 = fdlog_write_to_cache(data);
        if (!ret1) {
            printf("write to cache failture!\n");
            return 0;
        }
    }
    else {
        int ret1 = fdlog_write_to_cache(data);
        if (!ret1) {
            printf("write to cache failture!\n");
            return 0;
        }
    }
    return 1;
}

