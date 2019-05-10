//
//  fd_log.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_log_h
#define fd_log_h
#include "fd_construct_data.h"

/////////////////////////////////////////////
////             FDLog v1.0.0            ////
/////////////////////////////////////////////

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
int fdlog_initialize_by_rsa(char* root, unsigned char* ctr);

/*
 * Function: fdlog
 * ----------------------------
 *   Returns whether the write log was successful
 *
 *   data: FD_Construct_Data object
 *
 *   returns: 1 or 0
 */
int fdlog(FD_Construct_Data *data);

/*
 * Function: fdlog_sync
 * ----------------------------
 *   Returns whether the sync cache to local log file was successful
 *
 *   returns: 1 or 0
 */
int fdlog_sync(void);

/*
 * Function: fdlog_open_debug
 * ----------------------------
 *
 *   Open debug mode
 *
 *   is_open: 0 or 1 ,close or open
 *
 */
void fdlog_open_debug(int is_open);

/*
 * Function: fdlog_save_recent_days
 * ----------------------------
 *
 *   How many days log was saved (除当天，比如 设置3天，那么算上今天 应该保留算今天的最近4天的日志)
 *
 *   num: save days number
 *
 */
void fdlog_save_recent_days(int num);

/*
 * Function: fdlog_set_logfile_max_size
 * ----------------------------
 *
 *  Notes: maxsize value Maximum set to 5*1024*1024 5M. Invalid for more than 5M
 *  Max size of logfile
 *
 */
void fdlog_set_logfile_max_size(int maxsize);

/*
 * Function: fdlog_log_folder_path
 * ----------------------------
 *
 *   Get log file path
 *
 */
void fdlog_log_folder_path(char* path);

#endif /* fd_log_h */

