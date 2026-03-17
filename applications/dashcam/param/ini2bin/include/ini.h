#ifndef __INI_H__
#define __INI_H__

#include "minIni.h"
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t  INI_GetBool(const char *Section, const char *Key,
                   int32_t  DefValue, const char *Filename);
long INI_GetLong(const char *Section, const char *Key,
                    long DefValue, const char *Filename);
float INI_GetFloat(const char *Section, const char *Key,
                    float DefValue, const char *Filename);
int32_t  INI_GetString(const char *Section, const char *Key,
                    const char *DefValue, char *Buffer,
                   int32_t  BufferSize, const char *Filename);
int32_t  INI_GetSection(int32_t  idx, char *Buffer,int32_t  BufferSize,
                    const char *Filename);
int32_t  INI_GetKey(const char *Section,int32_t  idx, char *Buffer,
                   int32_t  BufferSize, const char *Filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif	/* __INI_H__ */