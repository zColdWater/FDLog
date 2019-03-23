#include "fd_json_helper.h"
#include <stdlib.h>
#include <string.h>

fd_json_map *fd_create_json_map(void) {
    fd_json_map *item = malloc(sizeof(fd_json_map));
    if (NULL != item)
        memset(item, 0, sizeof(fd_json_map));
    return item;
}

int fd_is_empty_json_map(fd_json_map *item) {
    
    // 创建 fd_json_map 空内存
    fd_json_map temp;
    memset(&temp, 0, sizeof(fd_json_map));
    
    // 让传入的参数 与 空内存比较，如果结果 = 0，证明内存相同
    if (memcmp(item, &temp, sizeof(fd_json_map)) == 0) {
        return 1;
    }
    return 0;
}

void fd_add_item_string(fd_json_map *map, const char *key, const char *value) {
    if (NULL != map && NULL != value && NULL != key && strnlen(key, 128) > 0) { // 关键变量不为空
        fd_json_map *item = map;
        fd_json_map *temp = item;
        if (!fd_is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
                item = item->nextItem;
            }
            temp = fd_create_json_map();
            item->nextItem = temp;
        }
        if (NULL != temp) {
            temp->type = FD_JSON_MAP_STRING;
            temp->key = (char *) key;
            temp->valueStr = value;
        }
    }
}

void fd_add_item_number(fd_json_map *map, const char *key, double number) {
    if (NULL != map && NULL != key && strnlen(key, 128) > 0) {
        fd_json_map *item = map;
        fd_json_map *temp = item;
        if (!fd_is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
                item = item->nextItem;
            }
            temp = fd_create_json_map();
            item->nextItem = temp;
        }
        if (NULL != temp) {
            temp->type = FD_JSON_MAP_NUMBER;
            temp->key = (char *) key;
            temp->valueNumber = number;
        }
    }
}

void fd_add_item_bool(fd_json_map *map, const char *key, int boolValue) {
    if (NULL != map && NULL != key && strnlen(key, 128) > 0) {
        fd_json_map *item = map;
        fd_json_map *temp = item;
        if (!fd_is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
                item = item->nextItem;
            }
            temp = fd_create_json_map();
            item->nextItem = temp;
        }
        if (NULL != temp) {
            temp->type = FD_JSON_MAP_BOOL;
            temp->key = (char *) key;
            temp->valueBool = boolValue;
        }
    }
}

void fd_delete_json_map(fd_json_map *map) {
    if (NULL != map) {
        fd_json_map *item = map;
        fd_json_map *temp = NULL;
        do {
            temp = item->nextItem;
            free(item);
            item = NULL;
            item = temp;
        } while (NULL != item);
    }
}

void fd_inflate_json_by_map(cJSON *root, fd_json_map *map) {
    if (NULL != root && NULL != map) {
        fd_json_map *item = map;
        do {
            switch (item->type) {
                case FD_JSON_MAP_STRING:
                    if (NULL != item->valueStr) {
                        cJSON_AddStringToObject(root, item->key, item->valueStr);
                    }
                    break;
                case FD_JSON_MAP_NUMBER:
                    cJSON_AddNumberToObject(root, item->key, item->valueNumber);
                    break;
                case FD_JSON_MAP_BOOL:
                    cJSON_AddBoolToObject(root, item->key, item->valueBool);
                    break;
                default:
                    break;
            }
            item = item->nextItem;
        } while (NULL != item);
    }
}
