#ifndef __CMDMNG_H__
#define __CMDMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t CMDMNG_SendMqCmd(int32_t client_id, int32_t chn_id, int32_t cmd_id, int32_t arg_val);
int32_t CMDMNG_SendMqCmd_Str(int32_t client_id, int32_t chn_id, int32_t cmd_id, int32_t arg_val, const char *str);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif