#ifndef __VIDEOMD_H__
#define __VIDEOMD_H__

#include <unistd.h>

#include "appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** macro define */
#define MD_EINVAL APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_EINVAL)                      /**<parm error*/
#define MD_EINTER APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_EINTER)                      /**<intern error*/
#define MD_ENOINIT APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_ENOINIT)                    /**< no initialize*/
#define MD_EINITIALIZED APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_EINITIALIZED)          /**< already initialized */
#define MD_EREGISTEREVENT APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_ERRNO_CUSTOM_BOTTOM) /**<thread creat or join error*/
#define MD_ETHREAD APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_ERRNO_CUSTOM_BOTTOM + 1)    /**<thread creat or join error*/

#define MD_EREGISTER_EVENT APP_APPCOMM_ERR_ID(APP_MOD_MD, APP_ERRNO_CUSTOM_BOTTOM)

typedef enum EVENT_VIDEOMD_E {
    EVENT_MD_CHANGE = APPCOMM_EVENT_ID(APP_MOD_MD, 0),
    EVENT_MD_BUTT
} EVENT_VIDEOMD_E;

int32_t MD_Callback(int32_t id, int32_t event);
int32_t MD_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __VIDEOMD_H__ */