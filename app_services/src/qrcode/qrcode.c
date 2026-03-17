#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cvi_log.h"
#include "qrcode.h"

#define HAVE_QRCODE_LIB 1

#ifdef ENABLE_ZBAR
#include "zbar.h"
#include "error.h"
typedef struct qrcode{
    zbar_image_scanner_t *scanner;
    zbar_image_t *image;
}qrcode_s;

#else
#undef HAVE_QRCODE_LIB
#endif

typedef struct qrcode_ctx{
    qrcode_s qr;
    qrcode_attr_s attr;
    void *qrbuf;
}qrcode_ctx_s;

#ifndef HAVE_QRCODE_LIB
#error "Err : Not Found QRCODE decode lib !!!!!!!!!!"
#endif

static qrcode_ctx_s *g_qrcode_ctx = NULL;

#ifdef ENABLE_ZBAR

static int32_t zbar_init(qrcode_s *qr, uint32_t w, uint32_t h, void *buf)
{
    if(buf == NULL){
        return -1;
    }

    zbar_set_verbosity(10);
    qr->scanner = zbar_image_scanner_create();
    if(qr->scanner == NULL){
        CVI_LOGE("zbar_image_scanner_create failed");
        return -1;
    }

    qr->image = zbar_image_create();
    if(qr->image == NULL){
        zbar_image_scanner_destroy(qr->scanner);
        CVI_LOGE("zbar_image_create failed");
        return -1;
    }

    zbar_image_scanner_set_config(qr->scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    zbar_image_scanner_set_config(qr->scanner, ZBAR_QRCODE, ZBAR_CFG_POSITION, 1);

    zbar_image_set_format(qr->image, *(const unsigned long *)"Y800");
    zbar_image_set_size(qr->image, w, h);
    zbar_image_set_data(qr->image, buf, w * h, zbar_image_free_data);
    return 0;
}

static int32_t zbar_deinit(qrcode_s *qr)
{
    zbar_image_destroy(qr->image); /*qrBuf free here*/
    zbar_image_scanner_destroy(qr->scanner);
    return 0;
}

static uint32_t zbar_decode(qrcode_s *qr, char (*result)[QRCODE_RESULT_MAX_LEN], uint32_t *cnt)
{
    *cnt = 0;
    if (zbar_scan_image(qr->scanner, qr->image) > 0) {
        const zbar_symbol_t *symbol = zbar_image_first_symbol(qr->image);
        for (; symbol && *cnt < QRCODE_RESULT_MAX_CNT; symbol = zbar_symbol_next(symbol)) {
            const char *data = zbar_symbol_get_data(symbol);
            snprintf(result[*cnt], sizeof(result[*cnt]), data);
            (*cnt)++;
        }
    }
    return *cnt;
}
#endif

int32_t qrcode_init(qrcode_attr_s *attr)
{
    CVI_LOGI("qrcode_init start");
    int32_t ret = 0;
    if(g_qrcode_ctx != NULL){
        return 0;
    }

    g_qrcode_ctx = (qrcode_ctx_s *)malloc(sizeof(qrcode_ctx_s));
    if(!g_qrcode_ctx){
        return -1;
    }
    memset(g_qrcode_ctx, 0x0, sizeof(qrcode_ctx_s));

    g_qrcode_ctx->qrbuf = malloc(attr->w * attr->h);
    if(!g_qrcode_ctx->qrbuf){
        free(g_qrcode_ctx);
        g_qrcode_ctx = NULL;
        return -1;
    }

    g_qrcode_ctx->attr.w = attr->w;
    g_qrcode_ctx->attr.h = attr->h;

#ifdef ENABLE_ZBAR
    ret = zbar_init(&g_qrcode_ctx->qr, attr->w, attr->h, g_qrcode_ctx->qrbuf);
#endif
    CVI_LOGI("qrcode_init end");
    if(ret != 0){
        free(g_qrcode_ctx->qrbuf);
        free(g_qrcode_ctx);
        g_qrcode_ctx = NULL;
        return -1;
    }

    return 0;
}

int32_t qrcode_deinit(void)
{
    if(g_qrcode_ctx){
    #ifdef ENABLE_ZBAR
        zbar_deinit(&g_qrcode_ctx->qr);
    #else
        free(g_qrcode_ctx->qrbuf);
    #endif
        free(g_qrcode_ctx);
        g_qrcode_ctx = NULL;
    }
    return 0;
}

uint32_t qrcode_decode(void *data, char (*result)[QRCODE_RESULT_MAX_LEN], uint32_t *cnt)
{
    if(g_qrcode_ctx){
        uint32_t len = g_qrcode_ctx->attr.w * g_qrcode_ctx->attr.h;
        memcpy(g_qrcode_ctx->qrbuf, data, len);
    #ifdef ENABLE_ZBAR
        return zbar_decode(&g_qrcode_ctx->qr, result, cnt);
    #endif
    }

    return 0;
}