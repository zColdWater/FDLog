//
//  fd_aes_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_aes_helper_h
#define fd_aes_helper_h


/*
 * Function: fd_aes_encrypt
 * ----------------------------
 *
 *   in: prepare encrypt datasource
 *   out: output data on out point
 *   length: An integer multiple of 16
 *   iv: aes128 iv
 *
 */
void fd_aes_encrypt(unsigned char *in, unsigned char *out, int length, unsigned char *iv);


/*
 * Function: fd_open_mmap_file
 * ----------------------------
 *
 *   key: AES128 KEY[16]
 *   iv: AES128 IV[16]
 *
 */
void fd_aes_init_key_iv(const char *key, const char *iv);


/*
 * Function: fd_open_mmap_file
 * ----------------------------
 *
 *   iv: assign the initialized iv to this parameter
 *
 */
void fd_aes_inflate_iv(unsigned char *iv);


#endif /* fd_aes_helper_h */
