#include <string.h>
#include "cJSON.h"
#include "stdlib.h"
#include "fd_construct_data.h"
#include "fd_json_helper.h"
#include "fd_console_helper.h"
#include "fd_core_model.h"

static const char *fdlog_key = "c";                   // 类型: String 日志内容 例如: " 这里需要注意，我出现问题了 "
static const char *fdlog_type = "lt";                 // 类型: Int 日志类型 例如: 业务日志(2), 性能日志(1), 崩溃日志(0)
static const char *fdflag_key = "f";                  // 类型: String 日志模块 例如: VOM ，SO ，MER 等等
static const char *fdlog_level = "lv";                // 类型: Int 日志等级 例如: Info Warning Error
static const char *fdlocaltime_key = "l";             // 类型: String 日志记录时间 例如: 1558427368
static const char *fdthreadname_key = "n";            // 类型: String 日志线程名字 例如: FDLogThread
static const char *fdthreadid_key = "i";              // 类型: Int 日志线程ID 例如: 4 , 5, 10
static const char *fdismain_key = "m";                // 类型: Int 日志是否是主线程 例如: 1 , 0
static const char *fdlog_version = "v";               // 类型: Int 日志本地版本 例如: 1 , 2 , 3 , 4 等等

static const char *fdapp_net = "nt";                  // 类型: String 网络环境 例如: HRPD 4G 3G 2G GPRS 2.75G EDGE 等等
static const char *fduser_system_name = "sys_n";      // 类型: String 手机操作系统名称 例如: Android, iOS
static const char *fdapp_name = "an";                 // 类型: String App名字 例如: NioApp, NioMate, ToolKit 等等
static const char *fdapp_version = "av";              // 类型: String App版本 例如: 3.5.0 , 3.5.1, 4.0.0 等等
static const char *fdphone_brand = "pb";              // 类型: String 手机品牌 例如: Apple, HuaWei, ViVo 等等
static const char *fduser_id = "ui";                  // 类型: String 用户id 例如: 12700912, 12700913 等等
static const char *fduser_name = "un";                // 类型: String 用户名字 例如: YongPeng.Zhu, Tony.Zou 等等
static const char *fduser_device = "ud";              // 类型: String 用户设备详细信息 例如: "iPhoneX 12.2 MQA52CH/A" 等等
static const char *fduser_system_version = "sys_v";   // 类型: String 手机操作系统版本 例如: 12.1 , 10.1 等
static const char *fdidentifier = "uid";              // 类型: String 单条日志唯一id 建议用uuid


FD_Construct_Data *
fd_construct_json_data(char *log,
                       char *flag,
                       char *local_time,
                       char *thread_name,
                       long long thread_id,
                       int is_main,
                       int level,
                       int log_type,
                       char *app_net,
                       char *user_system_name,
                       char *app_name,
                       char *app_version,
                       char *phone_brand,
                       char *user_id,
                       char *user_name,
                       char *user_device,
                       char *user_system_version,
                       char *identifier) {
    
    FD_Construct_Data *construct_data = NULL;
    cJSON *root = NULL;
    fd_json_map *map = NULL;
    
    root = cJSON_CreateObject();
    map = fd_create_json_map();
    
    if (NULL != root) {
        if (NULL != map) {
            
            // 日志内容
            fd_add_item_string(map, fdlog_key, log);
            // 日志模块名字: Vom SO 等等
            fd_add_item_string(map, fdflag_key, flag);
            // 记录时间
            fd_add_item_string(map, fdlocaltime_key, local_time);
            // 线程名字
            fd_add_item_string(map, fdthreadname_key, thread_name);
            // 线程ID
            fd_add_item_number(map, fdthreadid_key, (double) thread_id);
            // 是否在主线程
            fd_add_item_bool(map, fdismain_key, is_main);
            // 日志版本: 1
            fd_add_item_string(map, fdlog_version, FD_VERSION_NUMBER);
            // 日志等级: Info(2) Warning(1) Error(0)
            fd_add_item_number(map, fdlog_level, level);
            // 日志类型: 业务日志 性能日志 等等
            fd_add_item_number(map, fdlog_type, log_type);
            // 网络环境: 2G 4G
            fd_add_item_string(map, fdapp_net, app_net);
            // 手机操作系统名字: Android
            fd_add_item_string(map, fduser_system_name, user_system_name);
            // 应用名字: NioApp
            fd_add_item_string(map, fdapp_name, app_name);
            // 应用版本: 4.0.0
            fd_add_item_string(map, fdapp_version, app_version);
            // 手机品牌: HuaWei
            fd_add_item_string(map, fdphone_brand, phone_brand);
            // 用户ID: 44r1d14412
            fd_add_item_string(map, fduser_id, user_id);
            // 用户名字: Yongpeng.Zhu
            fd_add_item_string(map, fduser_name, user_name);
            // 用户设备详细信息: iPhoneX 12.2 MQA52CH/A
            fd_add_item_string(map, fduser_device, user_device);
            // 用户系统版本: 12.1
            fd_add_item_string(map, fduser_system_version, user_system_version);
            // UUID单条日志文件唯一标识符: ASDFGHJKASDFGHSDF
            fd_add_item_string(map, fdidentifier, identifier);
            
            fd_inflate_json_by_map(root, map);
            
            // 生成Jsonstring
            char *back_data = cJSON_PrintUnformatted(root);
            
            fd_printf("FDLog: log content is %s \n",back_data);
            
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
                memset(temp_data, 0, length);
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
                    
                    free(construct_data);
                    construct_data = NULL;
                    fd_printf(
                            "fdlog: construct_json_data_clogan > malloc memory fail for temp_data\n");
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
