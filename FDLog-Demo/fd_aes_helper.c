//
//  fd_aes_helper.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#include "fd_aes_helper.h"
#include "aes.h"
#include <stdio.h>
#include <string.h>


static unsigned char KEY[16] = {0}; 
static unsigned char IV[16] = {0};

void fd_aes_encrypt(unsigned char *in, unsigned char *out, int length, unsigned char *iv) {
    mbedtls_aes_context context;
    mbedtls_aes_setkey_enc(&context, (unsigned char *) KEY, 128);
    mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_ENCRYPT, length, iv, in, out);
}

void fd_aes_init_key_iv(const char *key, const char *iv) {
    memcpy(KEY, key, 16); // 密码16位 三种密钥大小(key size)：128 bits、192 bits和256 bits
    memcpy(IV, iv, 16);
}

void fd_aes_inflate_iv(unsigned char *iv) {
    memcpy(iv, IV, 16);
}


