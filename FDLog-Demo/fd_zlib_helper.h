//
//  fd_zlib_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_zlib_helper_h
#define fd_zlib_helper_h

#include "fd_core_model.h"

#define LOGAN_CHUNK 16384 // 定义数据块大小 16KB


/*
 * Function: fd_init_zlib
 * ----------------------------
 *   Returns whether the init zlib relate property was successful
 *
 *
 *   returns: 1 or 0
 */
int fd_init_zlib(FDLOGMODEL **model);

/*
 * Function: fd_zlib_compress
 * ----------------------------
 *   Returns whether the compress data was successful
 *
 *   data: Prepare the compressed data
 *   data_len: data length
 *   type: Z_SYNC_FLUSH or Z_FINISH
 *
 *   returns: 1 or 0
 */
int fd_zlib_compress(FDLOGMODEL **model, char *data, int data_len, int type);

/*
 * Function: fd_zlib_end_compress
 * ----------------------------
 *
 *   returns: 1 or 0
 */
void fd_zlib_end_compress(FDLOGMODEL **model);

#endif /* fd_zlib_helper_h */
