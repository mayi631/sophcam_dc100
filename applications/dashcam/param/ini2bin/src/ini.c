#include "ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


int32_t  INI_GetBool(const char *Section, const char *Key,
                   int32_t  DefValue, const char *Filename)
{
    return ini_getbool(Section, Key, DefValue, Filename);
}

long INI_GetLong(const char *Section, const char *Key,
                    long DefValue, const char *Filename)
{
    return ini_getl(Section, Key, DefValue, Filename);
}

float INI_GetFloat(const char *Section, const char *Key,
                    float DefValue, const char *Filename)
{
    return ini_getf(Section, Key, DefValue, Filename);
}

int32_t  INI_GetString(const char *Section, const char *Key,
                    const char *DefValue, char *Buffer,
                   int32_t  BufferSize, const char *Filename)
{
    return ini_gets(Section, Key, DefValue, Buffer, BufferSize, Filename);
}

int32_t  INI_GetSection(int32_t  idx, char *Buffer,int32_t  BufferSize,
                    const char *Filename)
{
    return ini_getsection(idx, Buffer, BufferSize, Filename);
}

int32_t  INI_GetKey(const char *Section,int32_t  idx, char *Buffer,
                   int32_t  BufferSize, const char *Filename)
{
    return ini_getkey(Section, idx, Buffer, BufferSize, Filename);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */