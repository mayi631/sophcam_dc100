#ifndef __LIVEVIEWMNG_H__
#define __LIVEVIEWMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdint.h>

int32_t LIVEVIEWMNG_Switch(uint32_t val);
int32_t LIVEVIEWMNG_MoveUp(int32_t wndid);
int32_t LIVEVIEWMNG_MoveDown(int32_t wndid);
int32_t LIVEVIEWMNG_Mirror(uint32_t val);
int32_t LIVEVIEWMNG_Filp(uint32_t val);
int32_t LIVEVIEWMNG_AdjustFocus(int32_t wndid , char* ratio);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __RECORDMNG_H__ */