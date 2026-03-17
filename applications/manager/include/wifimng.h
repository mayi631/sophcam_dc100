#ifndef __WIFIMNG_H__
#define __WIFIMNG_H__

#include "appcomm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t WIFIMNG_Start(HAL_WIFI_CFG_S WifiCfg, char *pstDefaultssid);
int32_t WIFIMNG_Stop(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __WIFIMNG_H__ */