#ifndef fd_json_helper_h
#define fd_json_helper_h

#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define FD_JSON_MAP_STRING 1
#define FD_JSON_MAP_NUMBER 2
#define FD_JSON_MAP_BOOL 3


/**
 FD日志 JSON数据模型
 */
typedef struct json_map {
    char *key;
    const char *valueStr;
    double valueNumber;
    int valueBool;
    int type;
    struct json_map *nextItem;
} fd_json_map;


/**
 初始化 fd_json_map 结构体

 @return fd_json_map 结构体
 */
fd_json_map *fd_create_json_map(void);

/**
 判断 fd_json_map 是否为空

 @param item 数据结构
 @return 是否为空
 */
int fd_is_empty_json_map(fd_json_map *item);

/**
 新增StringValue

 @param map fd_json_map 结构体
 @param key 关键Key
 @param value 字符串值
 */
void fd_add_item_string(fd_json_map *map, const char *key, const char *value);

/**
 新增NumberValue

 @param map fd_json_map 结构体
 @param key 关键Key
 @param number Number
 */
void fd_add_item_number(fd_json_map *map, const char *key, double number);

/**
 新增BoolValue

 @param map fd_json_map 结构体
 @param key 关键Key
 @param boolValue bool
 */
void fd_add_item_bool(fd_json_map *map, const char *key, int boolValue);

/**
 释放map结构体

 @param item fd_json_map object
 */
void fd_delete_json_map(fd_json_map *item);

/**
 填充cJSON对象

 @param root cJSON 对象指针
 @param map fd_json_map 对象指针
 */
void fd_inflate_json_by_map(cJSON *root, fd_json_map *map);

#endif
