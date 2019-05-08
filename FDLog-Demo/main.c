//
//  main.c
//  FDLog-Demo
//
//  Created by Yongpeng Zhu 朱永鹏 on 2019/2/23.
//  Copyright © 2019 NIO. All rights reserved.
//

//#include <stdio.h>
//#include <unistd.h>
//#include <dirent.h>
//#include <time.h>
//#include <string.h>
//#include <stdlib.h>
//#include "fd_log.h"
//
//
//// random string
//static char *rand_string(char *str, size_t size)
//{
//    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
//    if (size) {
//        --size;
//        for (size_t n = 0; n < size; n++) {
//            int key = rand() % (int) (sizeof charset - 1);
//            str[n] = charset[key];
//        }
//        str[size] = '\0';
//    }
//    return str;
//}
//
//// random string wrapper
//char* rand_string_alloc(size_t size)
//{
//    char *s = malloc(size + 1);
//    if (s) {
//        rand_string(s, size);
//    }
//    return s;
//}
//
//
//int main(int argc, const char * argv[]) {
//
//    // 对外是小端
//    // 释放堆内存
//
//    // 当前地址
//    char cwd[1024];
//    if (getcwd(cwd, sizeof(cwd)) != NULL) {
//        printf("Current working dir: %s \n", cwd);
//    } else {
//        perror("getcwd() error \n");
//        return 1;
//    }
//
//    // 开始记录日志
//    char KEY[] = "0123456789012345";
//    char IV[] = "0123456789012345";
//    int success = fdlog_initialize_by_rsa(cwd, KEY, IV, 1);
//    if (!success) {
//        printf("fd_initialize_log failture! \n");
//        return 0;
//    }
//
//    // 是否开启Debug模式
//    fdlog_open_debug(0);
//    // 保存最近几天的日志
//    fdlog_save_recent_days(7);
//    // 日志最大尺寸
//    fdlog_set_logfile_max_size(1024*500); // 1MB
//
//    // 存储日志的文件夹路径
//    char *temp = (char *)calloc(1, 1024);
//    fdlog_log_folder_path(temp);
//    printf("temp: %s \n",temp);
//    free(temp);
//    temp = NULL;
//
//    // 写入日志
//    int i = 1;
//    while (i < 19999) {
//        char *log = rand_string_alloc(30);
//        int flag = 5;
//        long long localtime = 123123;
//        char thread_name[] = "main";
//        int thread_id = 1;
//        int is_main = 1;
//        int level = 0;
//        FD_Construct_Data *data = fd_construct_json_data(log, flag, localtime, thread_name,thread_id, is_main, level);
//        printf("i: %d\n",i);
//        fdlog(data);
//        i++;
//    }
//
//
//
//    return 0;
//}




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

int main()
{

    int ret=0;

    // random data generator
    mbedtls_entropy_context entropy;
    mbedtls_entropy_init( &entropy );

    // randomness with seed
    mbedtls_ctr_drbg_context ctr_drbg;
    char *personalization = "My RSA demo";
    mbedtls_ctr_drbg_init( &ctr_drbg );

    ret = mbedtls_ctr_drbg_seed( &ctr_drbg , mbedtls_entropy_func, &entropy,
                                (const unsigned char *) personalization,
                                strlen( personalization ) );
    if( ret != 0 )
    {
        // ERROR HANDLING CODE FOR YOUR APP
    }
    mbedtls_ctr_drbg_set_prediction_resistance( &ctr_drbg,
                                               MBEDTLS_CTR_DRBG_PR_ON );
    ////////////////////////////////////////////////////////////////////////
    
    // 上下文 pk
    mbedtls_pk_context pk;
    // 初始化 上下文pk
    mbedtls_pk_init( &pk );
    
    // 加密内容
    unsigned char to_encrypt[]="Ford_Mustang";
    unsigned char to_decrypt[MBEDTLS_MPI_MAX_SIZE];
    const unsigned char pub_key[]=
    "-----BEGIN PUBLIC KEY-----\r\n"
    "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC9Jc728q/SZskvJc28q7wJjN2l\r\n"
    "rY8kV6cDWTtw/TikPugRdnaCdU/hBfahMRsh0d6ccibV3pZe/Hbug5l1yxyykOSe\r\n"
    "gVJx/qWI2FU8LbLHK7XcSzR1n/CogzhVYIHTu7pxKYt2unvmKB+gThUYduHW68xp\r\n"
    "iba04vDfCw7KvSlTtQIDAQAB\r\n"
    "-----END PUBLIC KEY-----\r\n";


    const unsigned char prv_pwd[]="password";
    const unsigned char prv_key[]=
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIICXgIBAAKBgQC9Jc728q/SZskvJc28q7wJjN2lrY8kV6cDWTtw/TikPugRdnaC\r\n"
    "dU/hBfahMRsh0d6ccibV3pZe/Hbug5l1yxyykOSegVJx/qWI2FU8LbLHK7XcSzR1\r\n"
    "n/CogzhVYIHTu7pxKYt2unvmKB+gThUYduHW68xpiba04vDfCw7KvSlTtQIDAQAB\r\n"
    "AoGAYp8jEZmqWR8kyQOCCVzV13juXKNpHj7hoxpUpu4xKVpvcCN/WThHpQGR/av4\r\n"
    "BKND2fifDSZY6z/h1y0gx81WsVBPaLr5fVNgR4CLGH7kjCjigUtjIzJ7UpI9+ZCh\r\n"
    "YWMHyjBs1UL0QkhPZe9h1lQQetvfB0lKNuVVltoySyJQP0ECQQD/vssAPrqGR4G5\r\n"
    "HfgVvhFWWdQgnl6DuOQeRza+LBbM/ZPdiPfhhcpIpM65bFJ1Wigta4aiq64MLOYz\r\n"
    "MlNz0MRxAkEAvVYJAd2hsVZfR0rL8PYLHbeYMhrRIf9gYgfu9a5MM440S/rIB0Gh\r\n"
    "moc7CyC0CqJHYKpL8RQHx13KeJKf/fYVhQJBAPul2oyQLOvKWuwzgBSs5NRqKaA7\r\n"
    "FVdZzCW6/zPboEfvUNtRVlB0XJpkiQHNg8nzf8tJnb5dXjKez5ka8SDqERECQQCu\r\n"
    "dqnEG1qUE1emVMjJ1550mqlWehl9L1m72z2ZCyvSUdXksUhCT3q+7p88aL0eE1yc\r\n"
    "OS/TDDcCwW0BX3KnzGsVAkEAtn+JnKHFtHNBN+GgWSdvibeRTZE10GqomsYTUvWR\r\n"
    "Zu4Zb+zMdf/7HCspxo50dpZN6s3WfGUA0fNqYKi3NCsBPg==\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";

    if( ( ret = mbedtls_pk_parse_public_key( &pk, pub_key, sizeof(pub_key) ) ) != 0 )
    {
        printf( " failed\n ! mbedtls_pk_parse_public_keyfile returned -0x%04x\n", -ret );
        return -1;
    }

    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
    size_t olen = 0;

    printf( "\nGenerating the encrypted value\n" );
    fflush( stdout );

    // 加密
    if( ( ret = mbedtls_pk_encrypt( &pk, to_encrypt, sizeof(to_encrypt),
                                   buf, &olen, sizeof(buf),
                                   mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        printf( " failed\n ! mbedtls_pk_encrypt returned -0x%04x\n", -ret );
        return -1;
    }



    for(int idx=0; idx<strlen(buf); printf("%02x", buf[idx++]));
    printf ("\n");
    mbedtls_pk_context pk1;
    mbedtls_pk_init(&pk1);

    
    if( ( ret = mbedtls_pk_parse_key( &pk1, prv_key, sizeof(prv_key), prv_pwd, strlen(prv_pwd) ) ) != 0 )
    {
        printf( " failed\n ! mbedtls_pk_parse_keyfile returned -0x%04x\n", -ret );
        return -1;
    }

    unsigned char result[MBEDTLS_MPI_MAX_SIZE];
//    olen = 0;

    printf( "\nGenerating the decrypted value" );
    fflush( stdout );

    if( ( ret = mbedtls_pk_decrypt( &pk1, buf, olen, result, &olen, sizeof(result),
                                   mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        printf( " failed\n! mbedtls_pk_decrypt returned -0x%04x\n", -ret );
        return -1;
    }
    else
    {

        fflush( stdout );
        printf("\n\n%s----------------\n\n", result);
    }

    return 0;
}

