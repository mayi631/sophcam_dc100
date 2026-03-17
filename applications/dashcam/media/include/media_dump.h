#ifndef __MEDIA_DUMP_H__
#define __MEDIA_DUMP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef ENABLE_ISP_PQ_TOOL
int32_t MEDIA_DUMP_SetDumpRawAttr(int32_t sns_id);
int32_t MEDIA_DUMP_DumpRaw(const int32_t sns_id);
int32_t MEDIA_DUMP_DumpYuv(int32_t sns_id);
void MEDIA_DUMP_VIISPReset(bool bInit);
void MEDIA_DUMP_ReplayInit(void);
void MEDIA_DUMP_GetSizeStatus(bool *en);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif