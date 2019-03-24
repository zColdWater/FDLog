#include "fd_directory_helper.h"
#include "fd_console_helper.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>

#define FD_MAX_PATH 1024 //路径最大长度

extern char *log_folder_path;
extern long *log_file_len;
extern char *mmap_cache_file_path;
extern char *log_file_path;


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

char* get_current_date() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time[18];
    sprintf(time, "%04d%02d%02d", (tm.tm_year + 1900),(tm.tm_mon + 1),(tm.tm_mday));
    char* temp = (char *)malloc(18);
    strcpy(temp, time);
    return temp;
}

// 判断文件和目录是否存在
int fd_is_file_exist(const char *path) {
    int isExist = 0;
    if (NULL != path && strnlen(path, 1) > 0) {
        if (access(path, F_OK) == 0) {
            isExist = 1;
        }
    }
    return isExist;
}

// 根据路径创建目录
int fd_makedir(const char *path) {
    size_t beginCmpPath = 0;
    size_t endCmpPath = 0;
    size_t pathLen = strlen(path);
    char currentPath[FD_MAX_PATH] = {0};

    fd_printf("makedir > path : %s\n", path);
    //相对路径
    if ('/' != path[0]) {
        //获取当前路径
        getcwd(currentPath, FD_MAX_PATH);
        strcat(currentPath, "/");
        fd_printf("makedir > currentPath : %s\n", currentPath);
        beginCmpPath = strlen(currentPath);
        strcat(currentPath, path);
        if (path[pathLen - 1] != '/') {
            strcat(currentPath, "/");
        }
        endCmpPath = strlen(currentPath);
    } else {
        //绝对路径
        strcpy(currentPath, path);
        if (path[pathLen - 1] != '/') {
            strcat(currentPath, "/");
        }
        beginCmpPath = 1;
        endCmpPath = strlen(currentPath);
    }

    //创建各级目录
    for (size_t i = beginCmpPath; i < endCmpPath; i++) {
        if ('/' == currentPath[i]) {
            currentPath[i] = '\0';
            if (access(currentPath, F_OK) != 0) {
                if (mkdir(currentPath, 0777) == -1) {
                    return -1;
                }
            }
            currentPath[i] = '/';
        }
    }
    return 0;
}

char* look_for_last_logfile() {
    
    // 获取当天的日期字符串
    char* date = get_current_date();
    char* current_file_folder_name = (char *)calloc(1, 1024);
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
        fd_printf("%ld\n", d);
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
        fd_printf("logfile:%s \n",logfile);
        
        /// 遍历文件夹里面的文件名
        int is_same_name = 0;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (current_file_folder_name)) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                fd_printf ("%s\n", ent->d_name);
                if(strcmp(logfile,ent->d_name)==0) {
                    fd_printf("日志文件有重名\n");
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
            fd_printf("文件流打开失败!\n");
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


void remove_log_file(int save_recent_days_num, char* root_path) {
    
    if (save_recent_days_num < 0) {
        fd_printf("remove_log_file: save_recent_days_num can't less than zero or equal zero! \n");
        return;
    }
    
    char Dt[50];
    time_t now = time(NULL);
    now = now - (24*60*60*save_recent_days_num);
    struct tm *t = localtime(&now);
    sprintf(Dt,"%04d%02d%02d", t->tm_year+1900,t->tm_mon+1,t->tm_mday);
    
    long target_Dt = atol(Dt);

    char path[FD_MAX_PATH];
    strcpy(path, root_path);
    strcat(path, "/");
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            long d = atol(ent->d_name);
            if ( d == 0 ) {
                printf("remove_log_file: is invalid file! \n");
                continue;
            }
            if (d < target_Dt) {
                // remove this folder
                char remove_path[FD_MAX_PATH];
                strcpy(remove_path, path);
                strcat(remove_path, ent->d_name);
                if (fd_is_file_exist(remove_path)) {
                    if(remove(remove_path) == 0) {
                        printf("remove path: %s \n",remove_path);
                    }
                    else {
                        perror("remove_log_file remove");
                    }
                }
            }
        }
        closedir (dir);
        dir = NULL;
        ent = NULL;
    } else {
        perror ("remove_log_file:");
    }
    
}

void remove_file(char* path) {
    if (fd_is_file_exist(path)) {
        if(remove(path) == 0) {
            printf("remove path: %s \n",path);
        }
        else {
            perror("remove");
        }
    }
}

