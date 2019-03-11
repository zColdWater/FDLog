//
//  fd_zlib_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_zlib_helper_h
#define fd_zlib_helper_h

#include <stdbool.h>
#include "fd_core_model.h"

#define LOGAN_CHUNK 16384 // 定义数据块大小 16KB

/// Compress State
#define FD_ZLIB_NONE 0
#define FD_ZLIB_INIT 1
#define FD_ZLIB_ING  2
#define FD_ZLIB_END  3
#define FD_ZLIB_FAIL 4


bool fd_init_zlib(FDLOGMODEL *model);
bool fd_zlib_compress(FDLOGMODEL *model, char *data, int data_len, int type);
void fd_zlib_end_compress(FDLOGMODEL *model);

#endif /* fd_zlib_helper_h */
