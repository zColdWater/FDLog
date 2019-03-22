//
//  fd_aes_helper.h
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

#ifndef fd_aes_helper_h
#define fd_aes_helper_h


/** DEMO
unsigned char plain[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
unsigned char cipher[16]={0};
unsigned char iv[] = "0123456789012345";

printf("plain char sizeof: %lu \n",sizeof(plain));
printf("cipher char sizeof: %lu \n",sizeof(cipher));

fd_aes_init_key_iv("0123456789012345", "0123456789012345");
fd_aes_encrypt(plain, cipher, 16, iv);

printf("encrypt content: %s \n", cipher);
**/


/**
 AES 加密

 @param in 待加密的内容
 @param out 输出的加密内容
 @param length 加密的数据长度(必须是16的整数倍)
 @param iv 加密向量
 */
void fd_aes_encrypt(unsigned char *in, unsigned char *out, int length, unsigned char *iv);


/**
 AES 初始化

 @param key 密钥
 @param iv 向量
 */
void fd_aes_init_key_iv(const char *key, const char *iv);


/**
 AES 静态变量 KEY IV 赋值

 @param iv 向量
 */
void fd_aes_inflate_iv(unsigned char *iv);


#endif /* fd_aes_helper_h */
