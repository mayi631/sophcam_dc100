#ifndef __RECORDMNG_H__
#define __RECORDMNG_H__

#include "appcomm.h"
#include "recorder.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** Error Code */
#define RECMNG_EINVAL          APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_EINVAL)                  /**<Invalid argument */
#define RECMNG_ENOTINIT        APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_ENOINIT)                 /**<Not inited */
#define RECMNG_EINITIALIZED    APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_EINITIALIZED)            /**<Already Initialized */
#define RECMNG_EINTER          APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_EINTER)                  /**<Already Initialized */
#define RECMNG_EREGISTER_EVENT APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_ERRNO_CUSTOM_BOTTOM)     /**<register event failed */
#define RECMNG_EMAXINSTANCE    APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_ERRNO_CUSTOM_BOTTOM + 1) /**<beyond maximum instance */
#define RECMNG_ESTORAGE        APP_APPCOMM_ERR_ID(APP_MOD_RECMNG, APP_ERRNO_CUSTOM_BOTTOM + 2) /**<storage interface error */

/** event ID define */
typedef enum EVENT_RECMNG_E {
    EVENT_RECMNG_STARTREC = APPCOMM_EVENT_ID(APP_MOD_RECMNG, 0),
    EVENT_RECMNG_STOPREC,
    EVENT_RECMNG_SPLITSTART,
    EVENT_RECMNG_SPLITREC,
    EVENT_RECMNG_STARTEVENTREC,
    EVENT_RECMNG_EVENTREC_END,
    EVENT_RECMNG_STARTEMRREC,
    EVENT_RECMNG_EMRREC_END,
    EVENT_RECMNG_OPEN_FAILED,
    EVENT_RECMNG_WRITE_ERROR,
    EVENT_RECMNG_PIV_START,
    EVENT_RECMNG_PIV_END,
    EVENT_RECMNG_BUTT
} EVENT_RECMNG_E;

int32_t RECORDMNG_EventCallBack(RECORDER_EVENT_E event_type, const char *filename, void *param);
int32_t RECORDMNG_ContCallBack(RECORDER_EVENT_E event_type, const char *filename, void *param);
int32_t RECORDMNG_GetSubtitleCallBack(void *p, char *str, int32_t str_len);
int32_t RECORDMNG_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __RECORDMNG_H__ */