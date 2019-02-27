//
//  fd_log.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/27.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_log_h
#define fd_log_h

#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include "fd_logmodel.h"


/////////////////////////////////////////////
////       FDLog 日志 API V1.0.0          ////
/////////////////////////////////////////////


int fdlog_init(const char *cache_dirs,
               const char *path_dirs,
               int max_file,
               const char *encrypt_key16,
               const char *encrypt_iv16);




#endif /* fd_log_h */





