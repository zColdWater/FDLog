#include "fd_rsa_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pk.h"
#include "rsa.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "base64.h"




//const unsigned char pub_key[]=
//"-----BEGIN PUBLIC KEY-----\r\n"
//"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCRwBcxeI0LTFJrBevaMSV2B5mj\r\n"
//"WF51b/VAmAb76L1IVQJx1JjCSI25G3P5omdPzS7Mbe2rlyHwOWjS3A6V6YiEYtwh\r\n"
//"JcAM7Z+gbwzCbjPSd/N+ONrmCwJcmj5xQky1prvtZhfxRRdd89fHm8yZ9JKO/kpX\r\n"
//"R/v2BSDl+q89aQmxmwIDAQAB\r\n"
//"-----END PUBLIC KEY-----\r\n";

static const unsigned char prv_pwd[]="nonpassword";
static const unsigned char prv_key[]=
"-----BEGIN PRIVATE KEY-----\r\n"
"MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAKv/k0wOVGRCLQIG\r\n"
"DMnJWqzEuWAiYa1SwD9Dz4eJEVdPXkDFfJMnHI5XtULUDWd39mMR8L+yZ8Wam4nB5\r\n"
"iICUGfFJr2OhjrHVHb3OGKFtAQUKPNI4ffYpTxdOAIXAf0sLYmAkoCwSMgMZXs055\r\n"
"iTolLoteC5+5wrPIMjEVQY5xf3AgMBAAECgYA4dQPcizeYUUCE0YeRFM72tvxCWgb\r\n"
"F5cvE+HU+f/d5OUwu5qQliUkOUCvna8Oamv39Nv5XbjAx5LX5WfQ4ZZoOaiRCWCFI\r\n"
"owRBl2Bz6SxKAIA2LMyX0aRk+BSTNINTqbjB3uu0oJqRoGmTfVJgUKTCsmGBnp3Ck\r\n"
"jJDcRdZa8SsQQJBAPiKb1by7/19ZXYErBTYxfcU5/gXwWRGGHMlcU/ILE4g+7okgF\r\n"
"yA56z7qafXYPnYmVfkGJSgEMO3FmJE8PTyFdUCQQCxKQ6RyMlzt/o7ZOSwo8wA95g\r\n"
"n6j22AufJx8rWwufYqkMLewB3ZWkMW8b/JDzlaelFv+5qe2xea14NTuFXlGCbAkEA\r\n"
"0rNRdm3XpJeU1ztYwweOtEvF594DtkVyXtOEPzWc0dNGdGSTaVhij5R+HGsLrGPCg\r\n"
"+88I2ubKsBhOoflmlJMzQJACuhPOFSXaqOCKZP9rI55KfoUiSiX4hPtt3OwnmZjpe\r\n"
"hdtmA9WByLSjKAXwcZJLrPnPbDNBUjR/DuWn1nzF+bTwJBAO4qK4taJI2K8ry11qG\r\n"
"/jcy85RAQpkK+pOci7BjPJ+jvDBpr3lzVacIiQklEHLaFefaNY//aXAV1NSXrBfcScsk=\r\n"
"-----END PRIVATE KEY-----\r\n";

int fd_rsa_decode(unsigned char *ctr, unsigned char *output, size_t output_length) {
    int ret=0;

    // random data generator
    mbedtls_entropy_context entropy;
    mbedtls_entropy_init( &entropy );
    
    // randomness with seed
    mbedtls_ctr_drbg_context ctr_drbg;
    char *personalization = "FDLog";
    mbedtls_ctr_drbg_init( &ctr_drbg );
    
    ret = mbedtls_ctr_drbg_seed( &ctr_drbg , mbedtls_entropy_func, &entropy,
                                (const unsigned char *) personalization,
                                strlen( personalization ) );
    if( ret != 0 )
    {
        // ERROR HANDLING CODE FOR YOUR APP
        printf("ERROR HANDLING CODE FOR YOUR APP");
    }
    mbedtls_ctr_drbg_set_prediction_resistance( &ctr_drbg,
                                               MBEDTLS_CTR_DRBG_PR_ON );
    ////////////////////////////////////////////////////////////////////////
    unsigned char data[MBEDTLS_MPI_MAX_SIZE];
    memset(data, 0, MBEDTLS_MPI_MAX_SIZE);
    // base64 decode 后字节数
    size_t olen = 0;
    int ret1 = mbedtls_base64_decode((unsigned char*)data, sizeof(data), &olen, ctr, strlen(ctr));
    if (ret1 != 0) {
        printf( " failed\n ! mbedtls_base64_decode returned %d \n",ret1);
        return -1;
    }
    
    mbedtls_pk_context pk1;
    mbedtls_pk_init(&pk1);
    if( ( ret = mbedtls_pk_parse_key( &pk1, prv_key, sizeof(prv_key), prv_pwd, strlen(prv_pwd) ) ) != 0 )
    {
        printf( " failed\n ! mbedtls_pk_parse_keyfile returned -0x%04x\n", -ret );
        return -1;
    }
    
    fflush( stdout );
    
    if( ( ret = mbedtls_pk_decrypt( &pk1, data, olen, output, &output_length, MBEDTLS_MPI_MAX_SIZE,
                                   mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        printf( " failed\n! mbedtls_pk_decrypt returned -0x%04x\n", -ret );
        return -1;
    }
    else
    {
        fflush( stdout );
        return 0;
    }
}




