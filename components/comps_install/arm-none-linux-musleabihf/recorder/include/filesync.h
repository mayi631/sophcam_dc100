
#ifndef FILE_SYNC_H
#define FILE_SYNC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FILESYNC_EVENT_CALLBACK)(void *, char *, uint32_t, void *, uint32_t);

typedef struct FILESYNC_EVENT_CB_S{
    void *cb;
    void *hdl;
    uint32_t event;
    void *argv0;
    uint32_t argv1;
}FILESYNC_EVENT_CB_S;

void FILESYNC_Push(char *filename, FILESYNC_EVENT_CB_S *cb);
int32_t FILESYNC_Init(void);
int32_t FILESYNC_Deinit(void);

#ifdef __cplusplus
}
#endif
#endif