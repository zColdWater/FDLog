//
//  fd_log.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

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
#include "fd_console_helper.h"
#include "fd_base_helper.h"
#include "pk.h"
#include "fd_rsa_helper.h"

extern FDLOGMODEL *model;

#define FD_INIT_SUCCESS 1; // log init success
#define FD_INIT_FAILURE 0; // log init failture
static int is_init_ok = FD_INIT_FAILURE;
static int is_logging = 0; // 是否正在记录日志
static int is_sync_log = 0; // 是否正在同步缓存到日志

/*
 * Function: fix_cache_file_struct
 * ----------------------------
 *
 *   Fix cache file point of mmap_tailer_ptr and mmap_content_len_ptr value
 *
 *   Return: 0 or 1
 *
 */
int fix_cache_file_struct() {
    
    if (!model->mmap_content_len_ptr || !model->mmap_content_len_ptr) {
        fd_printf("FDLog: FDLog fix_cache_file_struct: no init");
        return 0;
    }
    
    int i = 1;
    int total_len = *model->mmap_content_len_ptr;
    printf("total_len: %d \n",total_len);
    while (i < total_len) {
        if ((*model->mmap_tailer_ptr) == FD_MMAP_WRITE_CONTENT_HEADER) {
           model-> mmap_tailer_ptr -= 1;
            if ((*model->mmap_tailer_ptr) == FD_MMAP_WRITE_CONTENT_TAILER) {
                model->mmap_tailer_ptr += 1;
                memset(model->mmap_tailer_ptr, 0, i);
                // exit
                return 1;
            }
            else {
                model->mmap_tailer_ptr += 1;
            }
        }
        // next time
        model->mmap_tailer_ptr -= 1;
        *model->mmap_content_len_ptr -= 1;
        printf("mmap_content_len_ptr: %d \n", *model->mmap_content_len_ptr);
        i += 1;
    }
    
    return 0;
}



/*
 * Function: fix_log_file_struct
 * ----------------------------
 *
 *   Remove invalid log section
 *
 *   Return: 0 or 1
 *
 */
int fix_log_file_struct(char *log_path) {
    
    FILE * fp = fopen(log_path, "ab+");
    if (fp == NULL)
    {
        perror("Could not open file");
        return 0;
    }
    else {
        fseek(fp, 0, SEEK_END);
        long long longBytes = ftell(fp); // len
        if (longBytes < 1) {
            fd_printf("fd_log: bytes length less than one byte %d \n",longBytes);
            return 0;
        }
        
        fseek(fp, 0, SEEK_SET);
        int* file_server_ver = (int*)calloc(1, 4);
        fread(file_server_ver, 1, sizeof(int), fp);
        
        fseek(fp, 4, SEEK_SET); // 从第4个字节开始读取，因为前4个字节保存服务器版本 Int值。
        long long logs_length = longBytes - 4;
        char logs[logs_length];
        fread(logs, sizeof(char), logs_length, fp);
        fclose(fp);
        ReverseArray(logs, logs_length);
        
        if (logs[0] == FD_MMAP_WRITE_CONTENT_TAILER) { // 日志协议完整，不需要修复 直接退出。
            return 1;
        }
        
        // 日志协议缺失，需要将无效的日志信息移除。
        int i = 0;
        while (i < logs_length) {
            char temp = logs[i];
            if (temp == FD_MMAP_WRITE_CONTENT_HEADER) { // 找到日志协议头
                temp = logs[i + 1];
                if (temp == FD_MMAP_WRITE_CONTENT_TAILER) { // 找到日志协议头的上一位 检查是否是协议尾，如果是协议尾 证明日志是连续有效的。

                    // 清空当前日志文件
                    FILE *file = fopen(log_path, "w");
                    if (!file) { perror("Could not open file"); }
                    fclose(file);
                    
                    // 反转日志信息
                    ReverseArray(logs, logs_length);
                    // 需要移除的多余日志部分字节数
                    int remove_len = i + 1;
                    char* logs_temp = (char *)calloc(logs_length - remove_len, sizeof(char));
                    memcpy(logs_temp, logs, logs_length - remove_len);

                    FILE * fp = fopen(log_path, "ab+");
                    int server_version_byte = 4;
                    if (fp != NULL) {
                        fwrite(file_server_ver, 1, server_version_byte, fp); // 写入server版本
                        fwrite(logs_temp, sizeof(char), longBytes - remove_len - server_version_byte, fp);
                    }
                    else { perror("Could not open file"); }
                    fclose(fp);
                    free(logs_temp);
                    return 1;
                }
            }
            i++;
        }
    }
    return 0;
}



/*
 * Function: fdlog_set_logfile_max_size
 * ----------------------------
 *
 *  Max size of logfile
 *
 */
void fdlog_set_logfile_max_size(unsigned long long maxsize) {
    if (is_init_ok && (maxsize < 1024*1024*5)) {
        model->max_logfix_size = maxsize;
    }
}



/*
 * Function: fdlog_log_folder_path
 * ----------------------------
 *
 *   Get log file path
 *
 */
void fdlog_log_folder_path(char* path) {
    if ((model != NULL) && (model->log_folder_path != NULL)) {
        strcpy(path, model->log_folder_path);
    }
}



/*
 * Function: fdlog_save_recent_days
 * ----------------------------
 *
 *   How many days log was saved
 *
 *   num: save days number
 *
 */
void fdlog_save_recent_days(int num) {
    if (is_init_ok) {
        model->save_recent_days_num = num;
    }
}



/*
 * Function: fdlog_console_output
 * ----------------------------
 *
 *   Console output
 *
 *   is_open: 0 or 1 ,close or open
 *
 */
void fdlog_open_debug(int is_open) {
    fd_set_debug(is_open);
}



/*
 * Function: fdlog_init_dirs
 * ----------------------------
 *
 *   Create fdlog relate folder
 *
 *   root_path: FDLog root path
 *
 *   Returns whether create folders was successful
 *
 *   returns: 1 or 0
 */
int fdlog_init_dirs(const char *root_path, FDLOGMODEL **model) {
    
    FDLOGMODEL* model1 = *model;
    
    if (!model1->is_init_global_vars) {
        fd_printf("FDLog: FDLog init_fdlog_dirs: is_init_global_vars is 0 ! \n");
        return 0;
    }
    
    char cache_folder_path[FD_MAX_PATH];
    memset(cache_folder_path, 0, FD_MAX_PATH);
    
    char cache_file_path[FD_MAX_PATH];
    memset(cache_file_path, 0, FD_MAX_PATH);
    
    char log_folder_path1[FD_MAX_PATH];
    memset(log_folder_path1, 0, FD_MAX_PATH);
    
    char path[FD_MAX_PATH];
    memset(path, 0, FD_MAX_PATH);
    strcat(path, root_path);
    strcat(path, "/");
    strcat(path, FD_LOG_FOLDER_NAME);
    
    strcpy(log_folder_path1, path);
    strcat(log_folder_path1, "/");
    strcat(log_folder_path1, FD_LOG_FILE_FOLDER_NAME);
    
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_FOLDER_NAME);
    
    strcpy(cache_folder_path, path);
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_NAME);
    strcpy(cache_file_path, path);
    
    if (fd_makedir(cache_folder_path) == 0) {
        memcpy(model1->mmap_cache_file_path, cache_file_path, FD_MAX_PATH);
        memcpy(model1->log_folder_path, log_folder_path1, FD_MAX_PATH);
        *model = model1;
        return 1;
    }
    
    return 0;
}



/*
 * Function: insert_mmap_file_header
 * ----------------------------
 *
 *   Assumble cache file header data format
 *
 *   Returns whether insert header into cache file was successful
 *
 *   returns: 1 or 0
 */
int fdlog_insert_mmap_file_header() {
    
    if (!model->is_bind_mmap) {
        fd_printf("FDLog: FDLog: mmap file not bind memory point! \n");
        return 0;
    }
    
    cJSON *root = NULL;
    fd_json_map *map = NULL;
    root = cJSON_CreateObject();
    map = fd_create_json_map();
    char *back_data = NULL;
    if (NULL != root) {
        if (NULL != map) {
            char* date = get_current_date();
            fd_add_item_string(map, FD_VERSION_KEY, FD_VERSION_NUMBER);
            fd_add_item_string(map, FD_DATE, date);
            fd_add_item_number(map, FD_SERVER_VER, model->server_ver);
            fd_add_item_number(map, FD_SIZE, FD_MMAP_LENGTH);
            fd_inflate_json_by_map(root, map);
            back_data = cJSON_PrintUnformatted(root);
            free(date);
            date = NULL;
        }
        cJSON_Delete(root);
        free(map);
        map = NULL;
    }
    
    // reset mmap cache file content to '0'
    if (model->mmap_ptr != NULL) {
        memset(model->mmap_ptr, 0, FD_MMAP_LENGTH);
        int len = (int)strlen(back_data);
        unsigned char *temp = model->mmap_ptr;
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
        *temp = FD_MMAP_TOTAL_LOG_LEN_HEADER;
        temp++;
        temp += sizeof(int);
        *temp = FD_MMAP_TOTAL_LOG_LEN_TAILER;
        temp ++;
        
        fd_printf("FDLog: header insert content is :%s \n",back_data);
        return 1;
    } else {
        fd_printf("FDLog: occur serious bug that model->is_bind_mmap is ture but mmap_ptr is NULL! \n");
        return 0;
    }
}



/*
 * Function: fdlog_is_can_read_cache
 * ----------------------------
 *   Returns whether the is can read cache file was successful
 *
 *   returns: 1 or 0
 */
int fdlog_is_valid_cache() {
    if (!(model->is_bind_mmap)){
        fd_printf("FDLog: mmap file is bind failture! \n");
        return 0;
    }
    unsigned char *temp = model->mmap_ptr;
    if (*temp == FD_MMAP_FILE_HEADER) {
        fd_printf("FDLog: READ FD_MMAP_FILE_HEADER ✅\n");
        temp += 1;
        int *temp_header_content_len = (int*)temp;
        temp += sizeof(int);
        temp += *temp_header_content_len;
        if (*temp == FD_MMAP_FILE_TAILER) {
            fd_printf("FDLog: READ FD_MMAP_FILE_TAILER ✅\n");
            temp += 1;
            if (*temp == FD_MMAP_TOTAL_LOG_LEN_HEADER) {
                fd_printf("FDLog: READ FD_MMAP_FILE_CONTENT_HEADER ✅\n");
                temp += 1;
                int *temp_content_len_ptr = (int *)temp;
                temp += sizeof(int);
                if (*temp == FD_MMAP_TOTAL_LOG_LEN_TAILER) {
                    fd_printf("FDLog: READ FD_MMAP_FILE_CONTENT_TAILER ✅\n");
                    temp += 1;
                    int mmap_header_len = 2 + sizeof(int) + *temp_header_content_len;
                    int mmap_content_len = 2 + sizeof(int) + *temp_content_len_ptr;
                    
                    unsigned char *temp_tailer_ptr = model->mmap_ptr;
                    temp_tailer_ptr += (mmap_header_len + mmap_content_len);
                    
                    char tailer_c = *(temp_tailer_ptr - 1);
                    if (tailer_c == FD_MMAP_TOTAL_LOG_LEN_TAILER || tailer_c == FD_MMAP_WRITE_CONTENT_TAILER) {
                        fd_printf("FDLog: FDLog mmap_header_len: %d \n",mmap_header_len);
                        fd_printf("FDLog: FDLog mmap_content_len: %d \n",mmap_content_len);
                        fd_printf("FDLog: FDLog mmap_header_len+mmap_content_len: %d \n",mmap_header_len + mmap_content_len);
                        fd_printf("FDLog: FDLog mmap cache file can read! \n");
                        return 1;
                    }
                }
            }
        }
    }
    fd_printf("FDLog: cache file is invalid! \n");
    return 0;
}


/*
 * Function: fdlog_update_cache_point_position
 * ----------------------------
 *   Returns update success or failture
 *
 *   returns: the int value 0 is failture 1 is success.
 */
int fdlog_update_cache_point_position(FDLOGMODEL **model) {
    
    FDLOGMODEL* model1 = *model;
    
    unsigned char *temp = model1->mmap_ptr;
    if (*temp == FD_MMAP_FILE_HEADER) {
        temp += 1;
        model1->mmap_header_content_len_ptr = (int*)temp;
        temp += sizeof(int);
        memcpy(model1->mmap_header_content_ptr, temp, *model1->mmap_header_content_len_ptr);
        temp += *model1->mmap_header_content_len_ptr;
        if (*temp == FD_MMAP_FILE_TAILER) {
            temp += 1;
            if (*temp == FD_MMAP_TOTAL_LOG_LEN_HEADER) {
                temp += 1;
                model1->mmap_content_len_ptr = (int*)temp;
                temp += sizeof(int);
                if (*temp == FD_MMAP_TOTAL_LOG_LEN_TAILER) {
                    temp += 1;
                    model1->mmap_tailer_ptr = model1->mmap_ptr;
                    int mmap_header_len = 2 + sizeof(int) + *model1->mmap_header_content_len_ptr;
                    int mmap_content_len = 2 + sizeof(int) + *model1->mmap_content_len_ptr;
                    model1->mmap_tailer_ptr += (mmap_header_len + mmap_content_len);
                    
                    if (!(fdlog_is_valid_cache())){ return 0; }
                    
                    return 1;
                }
            }
        }
    }
    
    return 0;
}


/*
 * Function: fdlog_write_to_cache
 * ----------------------------
 *   Returns whether the reset global var was successful
 *
 *   data: FD_Construct_Data data
 *
 *   returns: 1 or 0
 */
int fdlog_write_to_cache(FD_Construct_Data *data) {
    
    if(!fdlog_is_valid_cache()) {
        fd_printf("FDLog: FDLog fdlog_write_to_cache: file is invalid, can't read content! \n");
        return 0;
    }
    if (!is_init_ok) {
        fd_printf("FDLog: FDLog fdlog_write_to_cache: log init failture, can't write to cache! \n");
        return 0;
    }
    if (!model->is_ready_gzip) {
        fd_printf("FDLog: FDLog fdlog_write_to_cache: model->is_ready_gzip is false, reinit zlib! \n");
        if (!fd_init_zlib(&model)) {
            fd_printf("FDLog: FDLog fdlog_write_to_cache: fd_init_zlib failture! \n");
            return 0;
        }
    }
    
    // Tailer must be point to write tailer or header tailer
    // otherwise cache file struct not correct!
    if ((*(model->mmap_tailer_ptr - 1) == FD_MMAP_WRITE_CONTENT_TAILER) ||
        (*(model->mmap_tailer_ptr - 1) == FD_MMAP_TOTAL_LOG_LEN_TAILER)) {
        
        *model->mmap_tailer_ptr = FD_MMAP_WRITE_CONTENT_HEADER;
        model->mmap_tailer_ptr += 1;
        model->mmap_current_log_len_ptr = (int *)model->mmap_tailer_ptr;
        model->mmap_tailer_ptr += sizeof(int);
        
        if (fd_zlib_compress(&model,data->data, data->data_len, Z_SYNC_FLUSH)) {
            fd_zlib_end_compress(&model);
            fd_aes_inflate_iv(model->aes_iv);
            fd_printf("FDLog: FDLog fdlog_write_to_cache: success! \n");
            return 1;
        }
    }
    
    // Cache file struct wrong, look for last integrated log and then
    // restore mmap_tailer_ptr and mmap_content_len_ptr value.
    // very small probability.
    else {
        if(fix_cache_file_struct()) {
            fdlog_write_to_cache(data);
        }
        // reset cache file
        else {
            if (!fdlog_insert_mmap_file_header()) {
                fd_printf("FDLog: fdlog_write_to_cache: fdlog_insert_mmap_file_header failture! \n");
                return 0;
            }
            // rebind cache point position
            if (!fdlog_update_cache_point_position(&model)) {
                fd_printf("FDLog: fdlog_write_to_cache: fdlog_update_cache_point_position failture! \n");
                return 0;
            }
            return 1;
        }
    }
    return 0;
}


/*
 * Function: reset_global_var
 * ----------------------------
 *   Returns whether the reset global var was successful
 *
 *   returns: 1 or 0
 */
int fdlog_reset_global_var(FDLOGMODEL** model, int server_ver) {
    
    FDLOGMODEL* model1 = *model;
    
    if (model1 == NULL) {
        model1 = (FDLOGMODEL *)malloc(sizeof(FDLOGMODEL));
        memset(model1, 0, sizeof(FDLOGMODEL));
        model1->mmap_header_content_ptr = (char*)calloc(1, FD_MMAP_HEADER_CONTENT_LEN);
        model1->log_folder_path = (char*)calloc(1, FD_MAX_PATH);
        model1->mmap_cache_file_path = (char*)calloc(1, FD_MAX_PATH);
        model1->log_file_len = (long *)calloc(1, sizeof(long));
        model1->log_file_path = (char *)calloc(1, FD_MAX_PATH);
    }
    else {
        model1->is_ready_gzip = 0;
        model1->is_bind_mmap = 0;
        model1->cache_remain_data_len = 0;
        model1->is_init_global_vars = 0;
        model1->save_recent_days_num = FD_SAVE_RECENT_DAYS;
        model1->max_logfix_size = FD_MAX_LOG_SIZE;
        
        memset(model1->aes_iv, 0, 16);
        memset(model1->cache_remain_data, 0, 16);
        memset(model1->strm, 0, sizeof(z_stream));
        
        model1->mmap_ptr = NULL;
        model1->mmap_tailer_ptr = NULL;
        model1->mmap_content_len_ptr = NULL;
        model1->mmap_current_log_len_ptr = NULL;
        model1->mmap_header_content_len_ptr = NULL;
        model1->mmap_header_content_ptr = NULL;
        memset(model1->mmap_header_content_ptr, 0, FD_MMAP_HEADER_CONTENT_LEN);
        
        memset(model1->log_folder_path, 0, FD_MAX_PATH);
        memset(model1->mmap_cache_file_path,0,FD_MAX_PATH);
        memset(model1->log_file_len, 0, sizeof(long));
        memset(model1->log_file_path, 0, FD_MAX_PATH);
    }
    
    if ((model1 != NULL) &&
        (model1->log_folder_path != NULL) &&
        (model1->mmap_cache_file_path != NULL) &&
        (model1->log_file_path != NULL) &&
        (model1->log_file_len != NULL)) {
        model1->is_init_global_vars = 1;
        model1->save_recent_days_num = FD_SAVE_RECENT_DAYS;
        model1->max_logfix_size = FD_MAX_LOG_SIZE;
        model1->server_ver = server_ver;
        fd_printf("FDLog: reset_global_var success! \n");
        
        *model = model1;
        return 1;
    }
    
    *model = model1;
    fd_printf("FDLog: reset_global_var failture! \n");
    return 0;
}



/*
 * Function: fdlog_sync
 * ----------------------------
 *   Returns whether the sync cache to local log file was successful
 *
 *   returns: 1 or 0
 */
int fdlog_sync() {
    if (is_sync_log) {
        fd_printf("FDLog: FDLog fdlog_sync: other task sync log, you need waiting other task finish \n");
        return 0;
    }
    
    is_sync_log = 1;
    if (!is_init_ok) {
        fd_printf("FDLog: FDLog fdlog_sync: init failture! \n");
        is_sync_log = 0;
        return 0;
    }
    
    if (model->is_zlibing) {
        fd_printf("FDLog: FDLog fdlog_sync: zlibing data can't sync cache to local log! \n");
        is_sync_log = 0;
        return 0;
    }

    int is_new_logfile = 0;
    char* last_file_name = look_for_last_logfile();
    // 如果 找不到上一次的日志文件
    if (last_file_name == NULL) {
        if (create_new_current_date_logfile()) {
            is_new_logfile = 1;
        }
        else {
            fd_printf("FDLog: FDLog fdlog_sync: create new file failture! \n");
            is_sync_log = 0;
            return 0;
        }
    }
    
    // 如果 找到上一次打开文件 读取数据
    else {
        
        // Save last_file_name path to log_file_path
        memset(model->log_file_path, 0, FD_MAX_PATH);
        memcpy(model->log_file_path, last_file_name, FD_MAX_PATH);
        
        // open file stream use `ab+`
        FILE *file_temp = fopen(model->log_file_path, "ab+");
        if (NULL != file_temp) {
            fseek(file_temp, 0, SEEK_END);
            long longBytes = ftell(file_temp);
            // get log file length
            memcpy(model->log_file_len, &longBytes, sizeof(long));
            fclose(file_temp);
        } else {
            fd_printf("FDLog: FDLog fdlog_sync: fopen log file failture! \n");
            free(last_file_name);
            last_file_name = NULL;
            is_sync_log = 0;
            return 0;
        }
    }
    
    // 如果当前日志文件大小大于日志设定最大大小，重新创建新日志文件。
    if ((*model->log_file_len) > model->max_logfix_size) {
        fd_printf("FDLog: FDLog fdlog_sync: current log file size:%lu \n",*model->log_file_len);
        if (!is_new_logfile) {
            memset(model->log_file_path, 0, FD_MAX_PATH);
            memset(model->log_file_len, 0, sizeof(long));
            create_new_current_date_logfile();
            is_new_logfile = 1;
        }
        else {
            fd_printf("FDLog: FDLog fdlog_sync: new file length still more that FD_MAX_LOG_SIZE! This is bug!!! \n");
            is_sync_log = 0;
            return 0;
        }
    }
    
    FILE* stream;
    stream = fopen(model->log_file_path, "ab+");
    
    // 全新日志文件没有内容
    if (is_new_logfile) {
        if (stream == NULL) {
            fd_printf("FDLog: FDLog fdlog_sync: open file failture! \n");
            is_sync_log = 0;
            return 0;
        }
        // 插入 server 版本号
        fwrite((const void*) & model->server_ver,sizeof(int),1,stream);
    }
    
    // 之前有内容的日志文件
    else {
        fix_log_file_struct(model->log_file_path);
    }
    
    
    if (stream == NULL)
    {
        perror("Could not open file");
        is_sync_log = 0;
        return 0;
    }
    else {
        
        // read logfile server version
        int logfile_server_ver;
        fseek(stream, 0, SEEK_SET);
        fread(&logfile_server_ver, sizeof(int), 1, stream);
        fseek(stream, 0, SEEK_END);
        
        // 日志文件里的服务器版本 与 初始化服务器版本不一致
        if (model->server_ver != logfile_server_ver) {
            fclose(stream);
            
            FILE* stream1;
            if (!is_new_logfile) { // 存在日志文件
                // create new logfile
                if (!create_new_current_date_logfile()) {
                    fd_printf("FDLog: FDLog fdlog_sync: create new file failture! \n");
                    is_sync_log = 0;
                    return 0;
                }
                stream1 = fopen(model->log_file_path, "ab+");
                fwrite((const void*) & model->server_ver,sizeof(int),1,stream1);
            }
            else {
                stream1 = fopen(model->log_file_path, "ab+");
            }
            
            unsigned char* temp = model->mmap_tailer_ptr - *model->mmap_content_len_ptr;
            fwrite(temp, sizeof(char), *model->mmap_content_len_ptr, stream1);
            fflush(stream1);
            fclose(stream1);
            *model->log_file_len += *model->mmap_content_len_ptr; // 更新日志文件大小
        }
        else { // 日志文件服务器版本 与 初始化服务器版本一致
            unsigned char* temp = model->mmap_tailer_ptr - *model->mmap_content_len_ptr;
            fwrite(temp, sizeof(char), *model->mmap_content_len_ptr, stream);
            fflush(stream);
            fclose(stream);
            *model->log_file_len += *model->mmap_content_len_ptr; // 更新日志文件大小
        }
    }
    
    // Reset mmap cahce file
    if (!fdlog_insert_mmap_file_header()) {
        fd_printf("FDLog: FDLog fdlog_sync: fdlog_insert_mmap_file_header failture! \n");
        is_sync_log = 0;
        return 0;
    }
    if (!fdlog_update_cache_point_position(&model)) {
        fd_printf("FDLog: FDLog fdlog_sync: fdlog_update_cache_point_position failture! \n");
        is_sync_log = 0;
        return 0;
    }
    
    is_sync_log = 0;
    return 1;
}


/*
 * Function: fdlog_sync_no_init
 * ----------------------------
 *   Returns whether the sync cache to local log file was successful
 *
 *   Don't need to init before invoke fdlog_sync_no_init
 *
 *   returns: 1 or 0
 */
int fdlog_sync_no_init(int server_id) {
    
    if (model->is_zlibing) {
        fd_printf("FDLog: FDLog fdlog_sync: zlibing data can't sync cache to local log! \n");
        return 0;
    }
    
    int is_new_logfile = 0;
    char* last_file_name = look_for_last_logfile();
    // 如果 找不到上一次的日志文件
    if (last_file_name == NULL) {
        if (create_new_current_date_logfile()) {
            is_new_logfile = 1;
        }
        else {
            fd_printf("FDLog: FDLog fdlog_sync: create new file failture! \n");
            return 0;
        }
    }
    
    // 如果 找到上一次打开文件 读取数据
    else {
        
        // Save last_file_name path to log_file_path
        memset(model->log_file_path, 0, FD_MAX_PATH);
        memcpy(model->log_file_path, last_file_name, FD_MAX_PATH);
        
        // open file stream use `ab+`
        FILE *file_temp = fopen(model->log_file_path, "ab+");
        if (NULL != file_temp) {
            fseek(file_temp, 0, SEEK_END);
            long longBytes = ftell(file_temp);
            // get log file length
            memcpy(model->log_file_len, &longBytes, sizeof(long));
            fclose(file_temp);
        } else {
            fd_printf("FDLog: FDLog fdlog_sync: fopen log file failture! \n");
            free(last_file_name);
            last_file_name = NULL;
            return 0;
        }
    }
    
    // 如果当前日志文件大小大于日志设定最大大小，重新创建新日志文件。
    if ((*model->log_file_len) > model->max_logfix_size) {
        fd_printf("FDLog: FDLog fdlog_sync: current log file size:%lu \n",*model->log_file_len);
        if (!is_new_logfile) {
            memset(model->log_file_path, 0, FD_MAX_PATH);
            memset(model->log_file_len, 0, sizeof(long));
            create_new_current_date_logfile();
            is_new_logfile = 1;
        }
        else {
            fd_printf("FDLog: FDLog fdlog_sync: new file length still more that FD_MAX_LOG_SIZE! This is bug!!! \n");
            return 0;
        }
    }
    
    FILE* stream;
    stream = fopen(model->log_file_path, "ab+");
    
    // 全新日志文件没有内容
    if (is_new_logfile) {
        if (stream == NULL) {
            fd_printf("FDLog: FDLog fdlog_sync: open file failture! \n");
            return 0;
        }
        // 插入 server 版本号
        printf("server_ver: %d \n",server_id);
        fwrite((const void*) & server_id,sizeof(int),1,stream);
    }
    
    // 之前有内容的日志文件
    else {
        fix_log_file_struct(model->log_file_path);
    }
    

    if (stream == NULL)
    {
        perror("Could not open file");
        return 0;
    }
    else {
        
        // read logfile server version
        int logfile_server_ver;
        fseek(stream, 0, SEEK_SET);
        fread(&logfile_server_ver, sizeof(int), 1, stream);
        fseek(stream, 0, SEEK_END);
        
        // 日志文件里的服务器版本 与 初始化服务器版本不一致
        if (server_id != logfile_server_ver) {
            fclose(stream);
            
            FILE* stream1;
            if (!is_new_logfile) { // 存在日志文件
                // create new logfile
                if (!create_new_current_date_logfile()) {
                    fd_printf("FDLog: FDLog fdlog_sync: create new file failture! \n");
                    return 0;
                }
                stream1 = fopen(model->log_file_path, "ab+");
                fwrite((const void*) & server_id,sizeof(int),1,stream1);
            }
            else {
                stream1 = fopen(model->log_file_path, "ab+");
            }
            
            unsigned char* temp = model->mmap_tailer_ptr - *model->mmap_content_len_ptr;
            fwrite(temp, sizeof(char), *model->mmap_content_len_ptr, stream1);
            fflush(stream1);
            fclose(stream1);
            *model->log_file_len += *model->mmap_content_len_ptr; // 更新日志文件大小
        }
        else { // 日志文件服务器版本 与 初始化服务器版本一致
            unsigned char* temp = model->mmap_tailer_ptr - *model->mmap_content_len_ptr;
            fwrite(temp, sizeof(char), *model->mmap_content_len_ptr, stream);
            fflush(stream);
            fclose(stream);
            *model->log_file_len += *model->mmap_content_len_ptr; // 更新日志文件大小
        }
    }
    
    // Reset mmap cahce file
    if (!fdlog_insert_mmap_file_header()) {
        fd_printf("FDLog: FDLog fdlog_sync: fdlog_insert_mmap_file_header failture! \n");
        return 0;
    }
    if (!fdlog_update_cache_point_position(&model)) {
        fd_printf("FDLog: FDLog fdlog_sync: fdlog_update_cache_point_position failture! \n");
        return 0;
    }
    
    return 1;
}


/*
 * Function: fdlog_initialize_by_rsa
 * ----------------------------
 *   Returns whether the initialization was successful
 *
 *   root: FDLog Root Directory
 *   ctr: RSA Ctr
 *
 *   returns: 1 or 0
 */
int fdlog_initialize_by_rsa(char* root, unsigned char* ctr) {
    
    is_init_ok = FD_INIT_FAILURE;
    
    if ((root == NULL) || (ctr == NULL)) {
        fd_printf("FDLog: FDLog fdlog_initialize_by_rsa: root or ctr is NULL \n");
        return is_init_ok;
    }
    
    unsigned char result[MBEDTLS_MPI_MAX_SIZE];
    memset(result, 0, MBEDTLS_MPI_MAX_SIZE);
    int isRSADecodeSuccess = fd_rsa_decode(ctr,result,MBEDTLS_MPI_MAX_SIZE);
    if (isRSADecodeSuccess != 0) {
        fd_printf("FDLog: FDLog fdlog_initialize_by_rsa: fd_rsa_decode failture \n");
        return is_init_ok;
    }
    
    // read key iv version from server
    cJSON* croot = cJSON_Parse((char*)result); // 数组32
    fd_printf("FDLog: FDLog fdlog_initialize_by_rsa: cJSON type:%d \n",croot->type);
    if (croot->type != (1 << 5)) {
        // 注意：因为服务器给的是数组，所以这里校验数据类型，不是数组初始化失败。
        fd_printf("FDLog: FDLog fdlog_initialize_by_rsa: data not Array type \n");
        return is_init_ok;
    }
    int arr_size = cJSON_GetArraySize(croot);
    if (arr_size <= 0) {
        fd_printf("FDLog: FDLog fdlog_initialize_by_rsa: array size less than 1 \n");
        return is_init_ok;
    }
    
    cJSON* object = (cJSON *)calloc(1, sizeof(struct cJSON));
    cJSON* temp_obj = cJSON_GetArrayItem(croot, 0);
    memcpy(object, temp_obj, sizeof(struct cJSON));
    char* iv = (char*)calloc(1, 1024);
    char* key = (char*)calloc(1, 1024);
    int server_ver = cJSON_GetObjectItem(object,FD_SERVER_VER1)->valueint;
    strcpy(iv, cJSON_GetObjectItem(object,FD_AES_IV)->valuestring);
    strcpy(key, cJSON_GetObjectItem(object,FD_AES_KEY)->valuestring);
    cJSON_Delete(croot);
    
    size_t iv_len = strlen(iv);
    size_t key_len = strlen(key);

    // guard server_ver iv_len and key_len is correct format
    if (server_ver < 0 || iv_len != 16 || key_len != 16) {
        fd_printf("FDLog: FDLog fdlog_initialize: server_ver is %d，less than 0 or iv, key length wrong! \n", server_ver);
        return is_init_ok;
    }
    
    fd_printf("FDLog: iv: %s \n",iv);
    fd_printf("FDLog: key: %s \n",key);

    
    if (!fdlog_reset_global_var(&model,server_ver)) { return is_init_ok; }
    if (!fdlog_init_dirs(root,&model)) { return is_init_ok; }
    if (!fd_open_mmap_file(&model, model->mmap_cache_file_path, &model->mmap_ptr)) { return is_init_ok; }
    
    fd_aes_init_key_iv(key, iv);
    fd_aes_inflate_iv(model->aes_iv);
    
    if (!fd_init_zlib(&model)) { return is_init_ok; }
    if (!fdlog_update_cache_point_position(&model))
    // 缓存文件格式错误，无法读取内容，直接从新开始覆盖掉旧的Cache文件内容。
    {
        if (!fix_cache_file_struct()) { // 修复日志缓存文件失败 放弃错误的缓存日志，造成一部分日志丢失 因为结构错乱。
         
            if (!fdlog_insert_mmap_file_header()) {
                fd_printf("FDLog: insert_mmap_file_header failture! \n");
                return is_init_ok;
            }
            
        }
        
        else {  // 修复日志缓存文件成功
            
            // Read date on cache header as before date
            cJSON* croot = cJSON_Parse(model->mmap_header_content_ptr);
            char* cache_date = (char*)calloc(1, 1024);
            
            // Cache file store server version
            int cache_server_ver = cJSON_GetObjectItem(croot,FD_SERVER_VER)->valueint;
            
            strcpy(cache_date, cJSON_GetObjectItem(croot,FD_DATE)->valuestring);
            cJSON_Delete(croot);
            
            // Get current date as now
            char* current_date = get_current_date();
            
            long before = atol(cache_date);
            long now = atol(current_date);
            
            free(cache_date);
            cache_date = NULL;
            
            free(current_date);
            current_date = NULL;
            
            fd_printf("FDLog: before %ld  now %ld \n",before,now);
            
            if ((now > before) || (cache_server_ver != server_ver)) { // 缓存文件过期，不是当天的。 或者 服务器版本 不一致 意味着 key iv 不同
                if (!fdlog_sync_no_init(cache_server_ver)) {
                    fd_printf("FDLog: cache write to local log file failture! \n");
                    return is_init_ok;
                }
                
                if (!fdlog_insert_mmap_file_header()) {
                    fd_printf("FDLog: insert_mmap_file_header failture! \n");
                    return is_init_ok;
                }
            }
        }
        
        // rebind cache point position
        if (!fdlog_update_cache_point_position(&model)) {
            fd_printf("FDLog: fdlog_update_cache_point_position failture! \n");
            return is_init_ok;
        }
        
    }
    
    else {
        
        // Read date on cache header as before date
        cJSON* croot = cJSON_Parse(model->mmap_header_content_ptr);
        char* cache_date = (char*)calloc(1, 1024);
        
        // Cache file store server version
        int cache_server_ver = cJSON_GetObjectItem(croot,FD_SERVER_VER)->valueint;
        
        strcpy(cache_date, cJSON_GetObjectItem(croot,FD_DATE)->valuestring);
        cJSON_Delete(croot);
        
        // Get current date as now
        char* current_date = get_current_date();
        
        long before = atol(cache_date);
        long now = atol(current_date);
        
        free(cache_date);
        cache_date = NULL;
        
        free(current_date);
        current_date = NULL;
        
        fd_printf("FDLog: before %ld  now %ld \n",before,now);
        
        if ((now > before) || (cache_server_ver != server_ver)) { // 缓存文件过期，不是当天的。 或者 服务器版本 不一致 意味着 key iv 不同
            
            // write desk log file when cache file have log content
            if (*model->mmap_content_len_ptr > 0) {
                if (!fdlog_sync_no_init(cache_server_ver)) {
                    fd_printf("FDLog: cache write to local log file failture! \n");
                    return is_init_ok;
                }
            }
            
            if (!fdlog_insert_mmap_file_header()) {
                fd_printf("FDLog: insert_mmap_file_header failture! \n");
                return is_init_ok;
            }
        }
        
        // rebind cache point position
        if (!fdlog_update_cache_point_position(&model)) {
            fd_printf("FDLog: fdlog_update_cache_point_position failture! \n");
            return is_init_ok;
        }
    }
    
    remove_log_file(model->save_recent_days_num,model->log_folder_path);
    is_init_ok = FD_INIT_SUCCESS;
    return is_init_ok;
}


/*
 * Function: fdlog
 * ----------------------------
 *   Returns whether the write log was successful
 *
 *   data: FD_Construct_Data object
 *
 *   returns: 1 or 0
 */
int fdlog(FD_Construct_Data *data) {
    if (is_logging) {
        fd_printf("FDLog: other task logging, you need waiting other task finish. \n");
        return 0;
    }
    is_logging = 1;
    if (!is_init_ok) {
        fd_printf("FDLog: init failture!\n");
        is_logging = 0;
        return 0;
    }
    
    if (model->is_zlibing) {
        fd_printf("FDLog: can't write because already zlibing \n");
        is_logging = 0;
        return 0;
    }
    
    // when mmap cache file content length more than max_length scale
    // then cache content sync to local log file
    if (*model->mmap_content_len_ptr > (int)(FD_MMAP_LENGTH * FD_MAX_MMAP_SCALE)) {
        fd_printf("FDLog: cache content need sync to local log file %d \n",*model->mmap_content_len_ptr);
        if (!fdlog_sync()) {
            fd_printf("FDLog: sync cache to local failture! \n");
            is_logging = 0;
            return 0;
        }
    }

    if (!fdlog_write_to_cache(data)) {
        fd_printf("FDLog: write to cache failture! \n");
        is_logging = 0;
        return 0;
    }
    
    is_logging = 0;
    return 1;
}

