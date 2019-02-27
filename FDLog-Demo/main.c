//
//  main.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>
#include "fd_log.h"



// Person Model
//typedef struct
//{
//    char firstName[32];
//    char lastName[32];
//    char email[64];
//    int age;
//    float height;
//} PERSON;


int main(int argc, const char * argv[]) {
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s \n", cwd);
    } else {
        perror("getcwd() error \n");
        return 1;
    }
    
    char* cache_dirs = (char *)malloc(1024);
    memset(cache_dirs, 0, 1024);
    memcpy(cache_dirs, cwd, 1024);
    strcat(cache_dirs, "/缓存文件");
    printf("缓存文件夹路径: %s \n", cache_dirs);
    
    char* log_path_dirs = (char *)malloc(1024);
    memset(log_path_dirs, 0, 1024);
    memcpy(log_path_dirs, cwd, 1024);
    strcat(log_path_dirs, "/日志文件");
    printf("日志文件夹路径: %s \n", log_path_dirs);

    
    int max_file = 1024*100;
    
    const char key[16] = "0987654321654321";
    const char iv[16] = "0987654321654321";
    
    // 开启DEBUG
    fdlog_debug(1);
    int ret = fdlog_init(cache_dirs, log_path_dirs, max_file, key, iv);
    int ret1 = fdlog_open("日志文件");
    
    printf("init result: %d \n", ret);
    printf("open result: %d \n", ret1);
    
    
    
    
//    // cJSON Demos
//    char json_str[] = "{\"firstName\":\"Brett\"}";
//    cJSON* root = cJSON_Parse(json_str);
//    printf("root value: %s \n",root->valuestring);
//    printf("root key: %s \n",root->string);
//
//    cJSON* item = cJSON_GetObjectItem(root,"firstName");
//    printf("firstName value: %s \n",item->valuestring);
//    printf("firstName key: %s \n",item->string);
//
//    // 释放
//    cJSON_Delete(root);

    
//    char json_str1[] = "{\"person\":{\"firstName\":\"z\",\"lastName\":\"jadena\",\"email\":\"jadena@126.com\",\"age\":8,\"height\":1.17}}";
//    cJSON* root1 = cJSON_Parse(json_str1);
//    cJSON* object = cJSON_GetObjectItem(root1,"person");
//
//    cJSON* item;
//    PERSON person;
//    memset(&person, 0, sizeof(PERSON));
//
//    item = cJSON_GetObjectItem(object,"firstName");
//    memcpy(person.firstName,item->valuestring,strlen(item->valuestring));
//    item = cJSON_GetObjectItem(object,"lastName");
//    memcpy(person.lastName,item->valuestring,strlen(item->valuestring));
//    item = cJSON_GetObjectItem(object,"email");
//    memcpy(person.email,item->valuestring,strlen(item->valuestring));
//    item = cJSON_GetObjectItem(object,"age");
//    person.age=item->valueint;
//    item = cJSON_GetObjectItem(object,"height");
//    person.height = item->valuedouble;
//
//    // 释放
//    cJSON_Delete(root1);
//
//    printf("person firstName: %s \n", person.firstName);
//    printf("person lastName: %s \n", person.lastName);
//    printf("person email: %s \n",person.email);
//    printf("person age: %d \n", person.age);
//    printf("person height: %f \n", person.height);


//    char json_str2[] = "{\"people\":[{\"firstName\":\"z\",\"lastName\":\"Jason\",\"email\":\"bbbb@126.com\",\"height\":1.67},{\"lastName\":\"jadena\",\"email\":\"jadena@126.com\",\"age\":8,\"height\":1.17},  {\"email\":\"cccc@126.com\",\"firstName\":\"z\",\"lastName\":\"Juliet\",\"age\":36,\"height\":1.55}]}";
//    cJSON* root2 = cJSON_Parse(json_str2);
//    cJSON* people_arr = cJSON_GetObjectItem(root2, "people");
//    int len = cJSON_GetArraySize(people_arr);
//    printf("数组长度 %d \n", len);
//    cJSON* someone = cJSON_GetArrayItem(people_arr, 1);
//    cJSON* email = cJSON_GetObjectItem(someone, "email");
//    printf("email: %s \n", email->valuestring);
    
    
    return 0;
}
