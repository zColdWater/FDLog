//
//  fd_rsa_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/5/9.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_rsa_helper_h
#define fd_rsa_helper_h

#include <stdio.h>

/*
 * Function: fd_rsa_decode
 * ----------------------------
 *
 *   ctr: RSA加密后的字符串(Base64 String)
 *   output: 解密结果
 *   output_length: 长度
 *
 *   returns: if 0 is success, -1 is failture.
 *
 */
int fd_rsa_decode(unsigned char *ctr, unsigned char *output, size_t output_length);

#endif /* fd_rsa_helper_h */

