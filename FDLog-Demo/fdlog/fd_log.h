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
int fdlog_initialize(char* root, char* key, char* iv);

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

#endif /* fd_log_h */

