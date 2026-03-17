#ifndef _QRCODE_SER_H
#define _QRCODE_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QRCODE_RESULT_MAX_CNT (2)
#define QRCODE_RESULT_MAX_LEN (2048)

typedef struct qrcode_attr{
    uint32_t w;
    uint32_t h;
}qrcode_attr_s;


int32_t qrcode_init(qrcode_attr_s *attr);
int32_t qrcode_deinit(void);
uint32_t qrcode_decode(void *data, char (*result)[QRCODE_RESULT_MAX_LEN], uint32_t *cnt);

#ifdef __cplusplus
}
#endif
#endif
