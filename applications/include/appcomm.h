#ifndef __APPCOMM_H__
#define __APPCOMM_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define UVC_SCRIPTS_PATH "/etc"
/** Invalid FD */
#define APPCOMM_FD_INVALID_VAL (-1)

/** General String Length */
#define APPCOMM_COMM_STR_LEN (64)

/** Path Maximum Length */
#define APPCOMM_MAX_PATH_LEN (128)

/** FileName Maximum Length */
#define APPCOMM_MAX_FILENAME_LEN (64)

/** Pointer Check */
#define APPCOMM_CHECK_POINTER(p, errcode)    \
    do {                                        \
        if (!(p)) {                             \
            CVI_LOGE("pointer[%s] is NULL\n", #p); \
            return errcode;                     \
        }                                       \
    } while (0)

/** Expression Check */
#define APPCOMM_CHECK_EXPR(expr, errcode)  \
    do {                                      \
        if (!(expr)) {                        \
            CVI_LOGE("expr[%s] false\n", #expr); \
            return errcode;                   \
        }                                     \
    } while (0)

/** Expression Check With ErrInformation */
#define APPCOMM_CHECK_EXPR_WITH_ERRINFO(expr, errcode, errstring) \
    do {                                                             \
        if (!(expr)) {                                               \
            CVI_LOGE("[%s] failed\n", errstring);                       \
            return errcode;                                          \
        }                                                            \
    } while (0)

/** Return Result Check */
#define APPCOMM_CHECK_RETURN(ret, errcode)       \
    do {                                            \
        if (0 != ret) {                    \
            CVI_LOGE("Error Code: [0x%08X]\n\n", ret); \
            return errcode;                         \
        }                                           \
    } while (0)

/** Return Result Check With ErrInformation */
#define APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, errcode, errstring) \
    do {                                                              \
        if (0 != ret) {                                      \
            CVI_LOGE("[%s] failed[0x%08X]\n", errstring, ret);           \
            return errcode;                                           \
        }                                                             \
    } while (0)

/** Expression Check Without Return */
#define APPCOMM_CHECK_EXPR_WITHOUT_RETURN(expr, errstring) \
    do {                                                      \
        if ((expr)) {                                        \
            CVI_LOGE("[%s] failed\n", errstring);                \
        }                                                     \
    } while (0)

/** Range Check */
#define APPCOMM_CHECK_RANGE(value, min, max) (((value) <= (max) && (value) >= (min)) ? 1 : 0)

/** Memory Safe Free */
#define CVI_APPCOMM_SAFE_FREE(p) \
    do {                        \
        if (NULL != (p)) {      \
            free(p);            \
            (p) = NULL;         \
        }                       \
    } while (0)

/** Value Align */
#define APPCOMM_ALIGN(value, base) (((value) + (base)-1) / (base) * (base))

/** strcmp enum string and value */
#define APPCOMM_STRCMP_ENUM(enumStr, enumValue) strncmp(enumStr, #enumValue, APPCOMM_COMM_STR_LEN)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef MAX
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#ifndef SWAP
#define SWAP(a, b) (a = (a) + (b), b = (a) - (b), a = (a) - (b))
#endif

/** Mutex Lock */
#define MUTEX_INIT_LOCK(mutex)                   \
    do {                                            \
        (void)pthread_mutex_init(&mutex, NULL); \
    } while (0)
#define MUTEX_LOCK(mutex)                  \
    do {                                      \
        (void)pthread_mutex_lock(&mutex); \
    } while (0)
#define MUTEX_UNLOCK(mutex)                  \
    do {                                        \
        (void)pthread_mutex_unlock(&mutex); \
    } while (0)
#define MUTEX_DESTROY(mutex)                  \
    do {                                         \
        (void)pthread_mutex_destroy(&mutex); \
    } while (0)

/** Cond */
#define COND_INIT(cond)                                               \
    do {                                                                 \
        pthread_condattr_t condattr;                                     \
        (void)pthread_condattr_init(&condattr);                      \
        (void)pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC); \
        (void)pthread_cond_init(&cond, &condattr);                   \
        (void)pthread_condattr_destroy(&condattr);                   \
    } while (0)
#define COND_TIMEDWAIT(cond, mutex, usec)                  \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        (void)pthread_cond_timedwait(&cond, &mutex, &ts); \
    } while (0)
#define COND_TIMEDWAIT_WITH_RETURN(cond, mutex, usec, ret) \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        ret = pthread_cond_timedwait(&cond, &mutex, &ts);     \
    } while (0)

#define COND_WAIT(cond, mutex)                   \
    do {                                            \
        (void)pthread_cond_wait(&cond, &mutex); \
    } while (0)
#define COND_SIGNAL(cond)                  \
    do {                                      \
        (void)pthread_cond_signal(&cond); \
    } while (0)
#define COND_DESTROY(cond)                  \
    do {                                       \
        (void)pthread_cond_destroy(&cond); \
    } while (0)

/*************************************************************************
  typedef
*************************************************************************/

typedef int32_t APP_ERRNO;

/*************************************************************************
  common error code
*************************************************************************/

#define APP_ERRNO_COMMON_BASE  0
#define APP_ERRNO_COMMON_COUNT 256

#define APP_EUNKNOWN            (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 1)  /*  */
#define APP_EOTHER              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 2)  /*  */
#define APP_EINTER              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 3)  /*  */
#define APP_EVERSION            (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 4)  /*  */
#define APP_EPAERM              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 5)  /*  */
#define APP_EINVAL              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 6)  /*  */
#define APP_ENOINIT             (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 7)  /*  */
#define APP_ENOTREADY           (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 8)  /*  */
#define APP_ENORES              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 9)  /*  */
#define APP_EEXIST              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 10) /*  */
#define APP_ELOST               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 11) /*  */
#define APP_ENOOP               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 12) /*  */
#define APP_EBUSY               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 13) /*  */
#define APP_EIDLE               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 14) /*  */
#define APP_EFULL               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 15) /*  */
#define APP_EEMPTY              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 16) /*  */
#define APP_EUNDERFLOW          (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 17) /*  */
#define APP_EOVERFLOW           (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 18) /*  */
#define APP_EACCES              (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 19) /*  */
#define APP_EINTR               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 20) /*  */
#define APP_ECONTINUE           (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 21) /*  */
#define APP_EOVER               (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 22) /*  */
#define APP_ERRNO_COMMON_BOTTOM (APP_ERRNO)(APP_ERRNO_COMMON_BASE + 23) /*  */

/*************************************************************************
  custom error code
*************************************************************************/

#define APP_ERRNO_BASE          (APP_ERRNO)(APP_ERRNO_COMMON_BASE + APP_ERRNO_COMMON_COUNT)
#define APP_EINITIALIZED        (APP_ERRNO)(APP_ERRNO_BASE + 1) /*  */
#define APP_ERRNO_CUSTOM_BOTTOM (APP_ERRNO)(APP_ERRNO_BASE + 2) /*  */

#define COND_INIT(cond)                                               \
    do {                                                                 \
        pthread_condattr_t condattr;                                     \
        (void)pthread_condattr_init(&condattr);                      \
        (void)pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC); \
        (void)pthread_cond_init(&cond, &condattr);                   \
        (void)pthread_condattr_destroy(&condattr);                   \
    } while (0)
#define COND_TIMEDWAIT(cond, mutex, usec)                  \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        (void)pthread_cond_timedwait(&cond, &mutex, &ts); \
    } while (0)
#define COND_TIMEDWAIT_WITH_RETURN(cond, mutex, usec, ret) \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        ret = pthread_cond_timedwait(&cond, &mutex, &ts);     \
    } while (0)

#define COND_WAIT(cond, mutex)                   \
    do {                                            \
        (void)pthread_cond_wait(&cond, &mutex); \
    } while (0)
#define COND_SIGNAL(cond)                  \
    do {                                      \
        (void)pthread_cond_signal(&cond); \
    } while (0)
#define COND_DESTROY(cond)                  \
    do {                                       \
        (void)pthread_cond_destroy(&cond); \
    } while (0)

/** App Event BaseId : [28bit~31bit] unique */
#define APPCOMM_EVENT_BASEID (0x10000000L)

/** App Event ID Rule [ --base[4bit]--|--module[8bit]--|--event[20bit]--]
    * module : module enum value [APP_MOD_E]
    * event_code : event code in specified module, unique in module
 */
#define APPCOMM_EVENT_ID(module, event) \
    ((uint32_t)((APPCOMM_EVENT_BASEID) | ((module) << 20) | (event)))

/* Get module id from event id */
#define APPCOMM_MODULE_ID(event_id) \
    ((uint32_t)((event_id - APPCOMM_EVENT_BASEID) >> 20))

/** App Error BaseId : [28bit~31bit] unique */
#define APPCOMM_ERR_BASEID (0x80000000L)

/** App Error Code Rule [ --base[4bit]--|--module[8bit]--|--error[20bit]--]
    * module : module enum value [APP_MOD_E]
    * event_code : event code in specified module, unique in module
 */
#define APP_APPCOMM_ERR_ID(module, err) \
    ((int32_t)((APPCOMM_ERR_BASEID) | ((module) << 20) | (err)))

#define APP_NULL                0L
#define APP_SUCCESS             0
#define APP_FAILURE             (-1)
#define APP_FAILURE_ILLEGAL_PARAM (-2)
#define APP_TRUE                1
#define APP_FALSE               0

/** App Module ID */
typedef enum cviAPP_MOD_E {
    APP_MOD_MEDIA = 0,
    APP_MOD_RECMNG,
    APP_MOD_PHOTOMNG,
    APP_MOD_FILEMNG,
    APP_MOD_LIVESVR,
    APP_MOD_USBMNG,
    APP_MOD_STORAGEMNG,
    APP_MOD_KEYMNG,
    APP_MOD_GAUGEMNG,
    APP_MOD_GSENSORMNG,
    APP_MOD_LEDMNG,
    APP_MOD_SCENE,
    APP_MOD_WEBSRV,
    APP_MOD_CONFACCESS,
    APP_MOD_PM,
    APP_MOD_SYSTEM,
    APP_MOD_RAWCAP,
    APP_MOD_UPGRADE,
    APP_MOD_OSD,
    APP_MOD_MD,
    APP_MOD_MODEMNG,
    APP_MOD_PARAM,
    APP_MOD_NETCTRL,
    APP_MOD_UI,
    APP_MOD_GPSMNG,
    APP_MOD_ACCMNG,
    APP_MOD_USBCTRL,
    APP_MOD_ALGADAPTER_ADAS,
    APP_MOD_VIDEOANALYSIS_ADAS,
    APP_MOD_VIDEOPROCESS,
    APP_MOD_VOLMNG,
    APP_MOD_PLAYBACKMNG,
    APP_MOD_SPEECHMNG,
    APP_MOD_ADASMNG,
    APP_MOD_BUTT
} APP_MOD_E;

const char *event_topic_get_name(uint32_t topic);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif