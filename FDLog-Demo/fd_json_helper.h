#ifndef fd_json_helper_h
#define fd_json_helper_h

#include "cJSON.h"

#define FD_JSON_MAP_STRING 1
#define FD_JSON_MAP_NUMBER 2
#define FD_JSON_MAP_BOOL 3


typedef struct json_map {
    char *key;
    const char *valueStr;
    double valueNumber;
    int valueBool;
    int type;
    struct json_map *nextItem;
} fd_json_map;


fd_json_map *fd_create_json_map(void);

int fd_is_empty_json_map(fd_json_map *item);

void fd_add_item_string(fd_json_map *map, const char *key, const char *value);

void fd_add_item_number(fd_json_map *map, const char *key, double number);

void fd_add_item_bool(fd_json_map *map, const char *key, int boolValue);

void fd_delete_json_map(fd_json_map *item);

void fd_inflate_json_by_map(cJSON *root, fd_json_map *map);

#endif
