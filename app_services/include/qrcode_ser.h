#ifndef _QRCODE_SER_H
#define _QRCODE_SER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "mapi.h"

    typedef struct QRCODE_PARAM_S
    {
        uint32_t w;
        uint32_t h;
        MAPI_VPROC_HANDLE_T vproc;
        uint32_t vproc_chnid;
    } QRCODE_SERVICE_PARAM_S;

    typedef void *QRCODE_SERVICE_HANDLE_T;

    int32_t QRCode_Service_Create(QRCODE_SERVICE_HANDLE_T *hdl, QRCODE_SERVICE_PARAM_S *attr);
    int32_t QRCode_Service_Destroy(QRCODE_SERVICE_HANDLE_T hdl);

#ifdef __cplusplus
}
#endif
#endif
