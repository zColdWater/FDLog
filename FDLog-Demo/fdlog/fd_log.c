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



#define FD_INIT_SUCCESS 1; // FDLog init 成功
#define FD_INIT_FAILURE 0; // FDLog init 失败



extern FDLOGMODEL *model;
extern unsigned char *mmap_ptr;
extern unsigned char *mmap_tailer_ptr;
extern int *mmap_content_len_ptr;
extern int *mmap_current_log_len_ptr;
extern int *mmap_header_content_len_ptr;
extern char *mmap_header_content_ptr;
extern char *log_folder_path;
extern long *log_file_len;
extern char *mmap_cache_file_path;
extern char *log_file_path;

/// 初始化成功标志 1成功 0失败
static int is_init_ok = FD_INIT_FAILURE;


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
int fdlog_init_dirs(const char *root_path) {
    
    if (!model->is_init_global_vars) {
        printf("FDLog init_fdlog_dirs: is_init_global_vars is 0 ! \n");
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
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_FOLDER_NAME);
    
    strcpy(cache_folder_path, path);
    strcat(path, "/");
    strcat(path, FD_LOG_CACHE_NAME);
    strcpy(cache_file_path, path);
    
    if (fd_makedir(cache_folder_path) == 0) {
        memcpy(mmap_cache_file_path, cache_file_path, FD_MAX_PATH);
        memcpy(log_folder_path, log_folder_path1, FD_MAX_PATH);
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
        printf("FDLog: mmap file not bind memory point! \n");
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
            fd_add_item_number(map, FD_VERSION_KEY, FD_VERSION_NUMBER);
            fd_add_item_string(map, FD_DATE, date);
            fd_add_item_number(map, FD_SIZE, FD_MMAP_LENGTH);
            fd_inflate_json_by_map(root, map);
            back_data = cJSON_PrintUnformatted(root);
        }
        cJSON_Delete(root);
    }
    
    // reset mmap cache file content to '0'
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
        
        printf("FDLog: header insert content is :%s \n",back_data);
        return 1;
    } else {
        printf("FDLog: occur serious bug that model->is_bind_mmap is ture but mmap_ptr is NULL! \n");
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
        printf("FDLog: mmap file is bind failture! \n");
        return 0;
    }
    unsigned char *temp = mmap_ptr;
    if (*temp == FD_MMAP_FILE_HEADER) {
        printf("READ FD_MMAP_FILE_HEADER ✅\n");
        temp += 1;
        int *temp_header_content_len = (int*)temp;
        temp += sizeof(int);
        temp += *temp_header_content_len;
        if (*temp == FD_MMAP_FILE_TAILER) {
            printf("READ FD_MMAP_FILE_TAILER ✅\n");
            temp += 1;
            if (*temp == FD_MMAP_FILE_CONTENT_HEADER) {
                printf("READ FD_MMAP_FILE_CONTENT_HEADER ✅\n");
                temp += 1;
                int *temp_content_len_ptr = (int *)temp;
                temp += sizeof(int);
                if (*temp == FD_MMAP_FILE_CONTENT_TAILER) {
                    printf("READ FD_MMAP_FILE_CONTENT_TAILER ✅\n");
                    temp += 1;
                    int mmap_header_len = 2 + sizeof(int) + *temp_header_content_len;
                    int mmap_content_len = 2 + sizeof(int) + *temp_content_len_ptr;
                    
                    unsigned char *temp_tailer_ptr = mmap_ptr;
                    temp_tailer_ptr += (mmap_header_len + mmap_content_len);
                    
                    char tailer_c = *(temp_tailer_ptr - 1);
                    if (tailer_c == FD_MMAP_FILE_CONTENT_TAILER || tailer_c == FD_MMAP_FILE_CONTENT_WRITE_TAILER) {
                        printf("FDLog mmap_header_len: %d \n",mmap_header_len);
                        printf("FDLog mmap_content_len: %d \n",mmap_content_len);
                        printf("FDLog mmap_header_len+mmap_content_len: %d \n",mmap_header_len + mmap_content_len);
                        printf("FDLog mmap cache file can read! \n");
                        return 1;
                    }
                }
            }
        }
    }
    printf("FDLog: cache file is invalid! \n");
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
        printf("FDLog fdlog_write_to_cache cache: file is invalid, can't read content! \n");
        return 0;
    }
    
    if (!is_init_ok) {
        printf("FDLog fdlog_write_to_cache: log init failture, can't write to cache! \n");
        return 0;
    }
    
    if (!model->is_ready_gzip) {
        printf("FDLog fdlog_write_to_cache: model->is_ready_gzip is NULL, can't write to cahce! \n");
        if (!fd_init_zlib()) {
            printf("FDLog fdlog_write_to_cache: fd_init_zlib failture! \n");
            return 0;
        }
    }
    
    // Tailer must be point to write tailer or header tailer
    // otherwise cache file struct not correct!
    if ((*(mmap_tailer_ptr - 1) == FD_MMAP_FILE_CONTENT_WRITE_TAILER) ||
        (*(mmap_tailer_ptr - 1) == FD_MMAP_FILE_CONTENT_TAILER)) {
        
        *mmap_tailer_ptr = FD_MMAP_FILE_CONTENT_WRITE_HEADER;
        mmap_tailer_ptr += 1;
        mmap_current_log_len_ptr = (int *)mmap_tailer_ptr;
        mmap_tailer_ptr += sizeof(int);
        
        if (fd_zlib_compress(data->data, data->data_len, Z_SYNC_FLUSH)) {
            fd_zlib_end_compress();
            fd_aes_inflate_iv(model->aes_iv);
            printf("FDLog fdlog_write_to_cache: success! \n");
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
int fdlog_reset_global_var() {
    
    if (model == NULL) {
        model = (FDLOGMODEL *)malloc(sizeof(FDLOGMODEL));
        memset(model, 0, sizeof(FDLOGMODEL));
    }
    else {
        model->is_ready_gzip = 0;
        model->zlib_type = 0;
        model->is_bind_mmap = 0;
        model->cache_remain_data_len = 0;
        model->is_init_global_vars = 0;
        memset(model->aes_iv, 0, 16);
        memset(model->cache_remain_data, 0, 16);
        memset(model->strm, 0, sizeof(z_stream));
    }
    
    if (!mmap_header_content_ptr) { mmap_header_content_ptr = (char*)calloc(1, FD_MMAP_HEADER_CONTENT_LEN); }
    else { memset(mmap_header_content_ptr, 0, FD_MMAP_HEADER_CONTENT_LEN); }
    
    if (!log_folder_path) { log_folder_path = (char*)calloc(1, FD_MAX_PATH); }
    else { memset(log_folder_path, 0, FD_MAX_PATH); }
    
    if (!mmap_cache_file_path) { mmap_cache_file_path = (char*)calloc(1, FD_MAX_PATH); }
    else { memset(mmap_cache_file_path,0,FD_MAX_PATH); }
    
    if (!log_file_len) { log_file_len = (long *)calloc(1, sizeof(long)); }
    else { memset(log_file_len, 0, sizeof(long)); }
    
    if (!log_file_path) { log_file_path = (char *)calloc(1, FD_MAX_PATH); }
    else { memset(log_file_path, 0, FD_MAX_PATH); }
    
    if ((model != NULL) &&
        (log_folder_path != NULL) &&
        (mmap_cache_file_path != NULL) &&
        (log_file_path != NULL) &&
        (log_file_len != NULL)) {
        model->is_init_global_vars = 1;
        printf("FDLog: reset_global_var success! \n");
        return 1;
    }
    
    printf("FDLog: reset_global_var failture! \n");
    return 0;
}



/*
 * Function: fdlog_update_cache_point_position
 * ----------------------------
 *   Returns update success or failture
 *
 *   returns: the int value 0 is failture 1 is success.
 */
int fdlog_update_cache_point_position() {
    
    if (!(fdlog_is_valid_cache())){ return 0; }
    unsigned char *temp = mmap_ptr;
    if (*temp == FD_MMAP_FILE_HEADER) {
        temp += 1;
        mmap_header_content_len_ptr = (int*)temp;
        temp += sizeof(int);
        memcpy(mmap_header_content_ptr, temp, *mmap_header_content_len_ptr);
        temp += *mmap_header_content_len_ptr;
        if (*temp == FD_MMAP_FILE_TAILER) {
            temp += 1;
            if (*temp == FD_MMAP_FILE_CONTENT_HEADER) {
                temp += 1;
                mmap_content_len_ptr = (int*)temp;
                temp += sizeof(int);
                if (*temp == FD_MMAP_FILE_CONTENT_TAILER) {
                    temp += 1;
                    mmap_tailer_ptr = mmap_ptr;
                    int mmap_header_len = 2 + sizeof(int) + *mmap_header_content_len_ptr;
                    int mmap_content_len = 2 + sizeof(int) + *mmap_content_len_ptr;
                    mmap_tailer_ptr += (mmap_header_len + mmap_content_len);
                    return 1;
                }
            }
        }
    }
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
    if ((*log_file_len) > FD_MAX_LOG_SIZE) {
        printf("log_file_len: %lu \n",*log_file_len);
        // 如果没创建新文件 创建新文件保存日志，因为日志长度大于设定最大长度。
        if (!is_new_logfile) {
            memset(log_file_path, 0, FD_MAX_PATH);
            memset(log_file_len, 0, sizeof(long));
            create_new_logfile();
        }
    }
    
    int bind = fdlog_update_cache_point_position();
    printf("bind result:%d\n",bind);
    
    /// 开始写入日志到本地文件
    FILE* stream;
    stream = fopen(log_file_path, "ab+");

    unsigned char* temp = mmap_tailer_ptr - *mmap_content_len_ptr;
    fwrite(temp, sizeof(char), *mmap_content_len_ptr, stream);//写入到文件中
    fflush(stream);
    fclose(stream);
    *log_file_len += *mmap_content_len_ptr; //修改文件大小
    
    /// 重置缓存MMAP相关信息
    int ret = fdlog_insert_mmap_file_header();
    if (ret == 0) { return 0; }
    int ret1 = fdlog_update_cache_point_position();
    if (ret1 == 0) { return 0; }
    
    return 1;
}



/*
 * Function: fdlog_initialize
 * ----------------------------
 *   Returns whether the initialization was successful
 *
 *   root: FDLog Root Directory
 *   key: AES128 KEY[16]
 *   iv: AES128 IV[16]
 *
 *   returns: 1 or 0
 */
int fdlog_initialize(char* root, char* key, char* iv) {
    is_init_ok = FD_INIT_FAILURE;
    
    if (!fdlog_reset_global_var()) { return is_init_ok; }
    if (!fdlog_init_dirs(root)) { return is_init_ok; }
    if (!fd_open_mmap_file(model, mmap_cache_file_path, &mmap_ptr)) { return is_init_ok; }
    
    fd_aes_init_key_iv(key, iv);
    fd_aes_inflate_iv(model->aes_iv);
    
    if (!fd_init_zlib()) { return is_init_ok; }
    if (fdlog_update_cache_point_position()) {
        
        // Read date on cache header as before date
        cJSON* croot = cJSON_Parse(mmap_header_content_ptr);
        char* cache_date = (char*)calloc(1, 1024);
        strcpy(cache_date, cJSON_GetObjectItem(croot,FD_DATE)->valuestring);
        cJSON_Delete(croot);
        
        // Get current date as now
        char* current_date = get_current_date();
        
        long before = atol(cache_date);
        long now = atol(current_date);
        
        printf("FDLog: before %ld  now %ld \n",before,now);
        
        if (now > before) { // 缓存文件过期，不是当天的。
            if (!fdlog_sync()) {
                printf("FDLog: cache write to local log file failture! \n");
                return is_init_ok;
            }
            
            if (!fdlog_insert_mmap_file_header()) {
                printf("FDLog: insert_mmap_file_header failture! \n");
                return is_init_ok;
            }
        }
        
        // rebind cache point position
        if (!fdlog_update_cache_point_position()) {
            printf("FDLog: fdlog_update_cache_point_position failture! \n");
            return is_init_ok;
        }
        
    }
    // 缓存文件格式错误，无法读取内容，直接从新开始覆盖掉旧的Cache文件内容。
    else {
        
        if (!fdlog_insert_mmap_file_header()) {
            printf("FDLog: insert_mmap_file_header failture! \n");
            return is_init_ok;
        }
        
        // rebind cache point position
        if (!fdlog_update_cache_point_position()) {
            printf("FDLog: fdlog_update_cache_point_position failture! \n");
            return is_init_ok;
        }
    }
    
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
    
    if (!is_init_ok) {
        printf("FDLog: init failture!\n");
        return 0;
    }
    
    if (model->is_zlibing) {
        printf("FDLog: can't write because already zlibing \n");
        return 0;
    }
    
    // When mmap cache file content length more than max_length scale
    // then cache content sync to local log file
    if ((float)*mmap_content_len_ptr > (float)(FD_MMAP_LENGTH * FD_MAX_MMAP_SCALE)) {
        printf("FDLog: cache content need sync to local log file %f \n",(float)*mmap_content_len_ptr);
        if (!fdlog_sync()) {
            printf("FDLog: sync cache to local failture! \n");
            return 0;
        }
    }

    if (!fdlog_write_to_cache(data)) {
        printf("FDLog: write to cache failture! \n");
        return 0;
    }
    
    return 1;
}

