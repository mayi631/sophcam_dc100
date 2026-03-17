#ifndef __FILE_RECOVER_H__
#define __FILE_RECOVER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FileRecoverEventType
{
    FILE_RECOVER_EVENT_OPEN_FAILED,
    FILE_RECOVER_EVENT_RECOVER_START,
    FILE_RECOVER_EVENT_RECOVER_PROGRESS,
    FILE_RECOVER_EVENT_RECOVER_FAILED,
    FILE_RECOVER_EVENT_RECOVER_FINISHED,
} FILE_RECOVER_EVENT_TYPE_E;

typedef struct FileRecoverEvent
{
    FILE_RECOVER_EVENT_TYPE_E type;
    double value;
} FILE_RECOVER_EVENT_S;

typedef void* FILE_RECOVER_HANDLE_T;
typedef void (*FILE_RECOVER_EVENT_HANDLER)(FILE_RECOVER_EVENT_S*);
typedef void (*FILE_RECOVER_CUSTOM_ARG_EVENT_HANDLER)(void*, FILE_RECOVER_EVENT_S*);

int32_t FILE_RECOVER_Create(FILE_RECOVER_HANDLE_T *handle);
int32_t FILE_RECOVER_Destroy(FILE_RECOVER_HANDLE_T *handle);
int32_t FILE_RECOVER_Open(FILE_RECOVER_HANDLE_T handle, const char* input_file_path);
int32_t FILE_RECOVER_Check(FILE_RECOVER_HANDLE_T handle);
int32_t FILE_RECOVER_Dump(FILE_RECOVER_HANDLE_T handle);
int32_t FILE_RECOVER_Recover(FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time);
int32_t FILE_RECOVER_RecoverAsync(FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time);
int32_t FILE_RECOVER_RecoverJoin(FILE_RECOVER_HANDLE_T handle);
int32_t FILE_RECOVER_Close(FILE_RECOVER_HANDLE_T handle);
int32_t FILE_RECOVER_SetEventHandler(FILE_RECOVER_HANDLE_T handle,
        FILE_RECOVER_EVENT_HANDLER handler);
int32_t FILE_RECOVER_SetCustomArgEventHandler(FILE_RECOVER_HANDLE_T handle,
        FILE_RECOVER_CUSTOM_ARG_EVENT_HANDLER handler, void* custom_arg);
void FILE_RECOVER_PreallocateState(FILE_RECOVER_HANDLE_T handle, bool PreallocFlage);

#ifdef __cplusplus
}
#endif

#endif
