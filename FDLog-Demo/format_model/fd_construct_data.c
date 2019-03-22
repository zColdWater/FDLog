#include <string.h>
#include "cJSON.h"
#include "stdlib.h"
#include "fd_construct_data.h"
#include "fd_json_helper.h"
#include "fd_console_helper.h"

static const char *log_key = "c";
static const char *flag_key = "f";
static const char *localtime_key = "l";
static const char *threadname_key = "n";
static const char *threadid_key = "i";
static const char *ismain_key = "m";

FD_Construct_Data *
fd_construct_json_data(char *log, int flag, long long local_time, char *thread_name,
                           long long thread_id, int is_main) {
    
    FD_Construct_Data *construct_data = NULL;
    cJSON *root = NULL;
    fd_json_map *map = NULL;
    
    root = cJSON_CreateObject();
    map = fd_create_json_map();
    
    if (NULL != root) {
        if (NULL != map) {
            
            fd_add_item_string(map, log_key, log);
            fd_add_item_number(map, flag_key, (double) flag);
            fd_add_item_number(map, localtime_key, (double) local_time);
            fd_add_item_string(map, threadname_key, thread_name);
            fd_add_item_number(map, threadid_key, (double) thread_id);
            fd_add_item_bool(map, ismain_key, is_main);
            
            
            fd_inflate_json_by_map(root, map);
            
            // 生成Jsonstring
            char *back_data = cJSON_PrintUnformatted(root);
            // 为FD_Construct_Data申请堆内存
            construct_data = (FD_Construct_Data *) malloc(sizeof(FD_Construct_Data));
            
            if (NULL != construct_data) { // 申请成功
                // 清空 construct_data
                memset(construct_data, 0, sizeof(FD_Construct_Data));
                
                // 得到日志jsonstring的内容长度
                size_t str_len = strlen(back_data);
                // 长度+1 在后面拼加 '\n' 换行符号。
                size_t length = str_len + 1;
                unsigned char *temp_data = (unsigned char *) malloc(length);
                if (NULL != temp_data) {
                    
                    unsigned char *temp_point = temp_data;
                    memset(temp_point, 0, length);
                    memcpy(temp_point, back_data, str_len);
                    temp_point += str_len;
                    char return_data[] = {'\n'};
                    memcpy(temp_point, return_data, 1); //添加\n字符
                    construct_data->data = (char *) temp_data; //赋值
                    construct_data->data_len = (int) length;
                    
                } else {
                    
                    free(construct_data); //创建数据
                    construct_data = NULL;
                    fd_printf(
                            "construct_json_data_clogan > malloc memory fail for temp_data\n");
                }
            }
            
            free(back_data);
            back_data = NULL;
        }
        cJSON_Delete(root);
    }
    
    if (NULL != map) {
        fd_delete_json_map(map);
    }
    
    return construct_data;
}


void fd_construct_data_delete(FD_Construct_Data *item) {
    if (NULL != item) {
        if (NULL != item->data) {
            free(item->data);
            item->data = NULL;
        }
        free(item);
        item = NULL;
    }
}
