/**
* @file    keymng.h
* @brief   product keymng struct and interface
* @version   1.0

*/

#ifndef _KEYMNG_H
#define _KEYMNG_H

#include "appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     KEYMNG */
/** @{ */  /** <!-- [KEYMNG] */

/** macro define */
#define KEYMNG_KEY_NUM_EACH_GRP (4)                        /**<key number in group-key*/

#define KEYMNG_EINVAL            APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_EINVAL)    /**<parm invlid*/
#define KEYMNG_EINTER            APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_EINTER)          /**<intern error*/
#define KEYMNG_ENOINIT           APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_ENOINIT)  /**< no initialize*/
#define KEYMNG_EINITIALIZED      APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_EINITIALIZED) /**< already initialized */
#define KEYMNG_EREGISTEREVENT    APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define KEYMNG_ETHREAD           APP_APPCOMM_ERR_ID(APP_MOD_KEYMNG,APP_ERRNO_CUSTOM_BOTTOM+1)           /**<thread creat or join error*/

typedef enum EVENT_KEYMNG_E
{
    EVENT_KEYMNG_SHORT_CLICK = APPCOMM_EVENT_ID(APP_MOD_KEYMNG, 0),   /**<short key click event*/
    EVENT_KEYMNG_LONG_CLICK,    /**<long key click event*/
    EVENT_KEYMNG_HOLD_DOWN,     /**<key hold up event*/
    EVENT_KEYMNG_HOLD_UP,       /**<key hold down event*/
    EVENT_KEYMNG_GROUP,     /**<group key event*/
    EVENT_KEYMNG_BUIT
} EVENT_KEYMNG_E;


/** key index enum */
typedef enum KEYMNG_KEY_IDX_E
{
    KEYMNG_KEY_IDX_0 = 0,
    KEYMNG_KEY_IDX_1,
    KEYMNG_KEY_IDX_2,
    KEYMNG_KEY_IDX_3,
    KEYMNG_KEY_IDX_4,
    KEYMNG_KEY_IDX_5,
    KEYMNG_KEY_IDX_6,
    KEYMNG_KEY_IDX_BUTT,
} KEYMNG_KEY_IDX_E;

/** key type enum */
typedef enum KEYMNG_KEY_TYPE_E
{
    KEYMNG_KEY_TYPE_CLICK = 0, /**<support click and longclick event */
    KEYMNG_KEY_TYPE_HOLD,      /**<support keydown and keyup event */
} KEYMNG_KEY_TYPE_E;

/** click key attribute */
typedef struct KEYMNG_KEY_CLICK_ATTR_S
{
    bool bLongClickEnable; /**<ture: support click and longclick event; false: only support click event */
    uint32_t  u32LongClickTime_msec; /**<long click check time, valid when longclick enabled */
} KEYMNG_KEY_CLICK_ATTR_S;

/** hold key attribute */
typedef struct KEYMNG_KEY_HOLD_ATTR_S
{
} KEYMNG_KEY_HOLD_ATTR_S;

/** key attribute */
typedef struct KEYMNG_KEY_ATTR_S
{
    KEYMNG_KEY_TYPE_E enType;   /**<click type or hold type*/
    int32_t s32Id; /**<key id */
    union tagKEYMNG_KEY_ATTR_U
    {
        KEYMNG_KEY_CLICK_ATTR_S stClickKeyAttr;   /**<click attr type */
        KEYMNG_KEY_HOLD_ATTR_S stHoldKeyAttr;     /**<hold attr type */
    } unAttr;
} KEYMNG_KEY_ATTR_S;

/** key configure */
typedef struct KEYMNG_KEY_CFG_S
{
    uint32_t u32KeyCnt;     /**<key count*/
    KEYMNG_KEY_ATTR_S astKeyAttr[KEYMNG_KEY_IDX_BUTT];
} KEYMNG_KEY_CFG_S;


/** group-key configure */
typedef struct KEYMNG_GRP_KEY_CFG_S
{
    bool bEnable;    /**<ture: support group key event; false: not support */
    uint32_t au32GrpKeyIdx[KEYMNG_KEY_NUM_EACH_GRP]; /**<only support two keys group at present*/
} KEYMNG_GRP_KEY_CFG_S;

/** keymng configure */
typedef struct KEYMNG_CFG_S
{
    KEYMNG_KEY_CFG_S stKeyCfg;
    KEYMNG_GRP_KEY_CFG_S stGrpKeyCfg;
} KEYMNG_CFG_S;

/**
* @brief    register keymng event
* @return 0 success,non-zero error code.
*/
int32_t KEYMNG_RegisterEvent(void);

/**
* @brief    keymng initialization, create key event check task
* @param[in] pstCfg: keymng configure, including key/grpkey configure
* @return 0 success,non-zero error code.
*/
int32_t KEYMNG_Init(KEYMNG_CFG_S KeyCfg);

/**
* @brief    keymng deinitialization, destroy key event check task
* @return 0 success,non-zero error code.
*/
int32_t KEYMNG_DeInit(void);

/** @}*/  /** <!-- ==== KEYMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _KEYMNG_H */
