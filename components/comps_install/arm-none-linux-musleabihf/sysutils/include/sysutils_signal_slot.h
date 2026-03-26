#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SIGNAL_SLOT_TYPE_NONE,
    SIGNAL_SLOT_TYPE_VOID,
    SIGNAL_SLOT_TYPE_BOOL,
    SIGNAL_SLOT_TYPE_INT,
    SIGNAL_SLOT_TYPE_INT64,
    SIGNAL_SLOT_TYPE_FLOAT,
    SIGNAL_SLOT_TYPE_STRING,
    SIGNAL_SLOT_TYPE_INT_INT,
    SIGNAL_SLOT_TYPE_UINT32_UINT32
} SIGNAL_SLOT_TYPE_E;

typedef struct
{
    SIGNAL_SLOT_TYPE_E type;
    void *signal;
} SIGNAL_S;

typedef struct
{
    SIGNAL_SLOT_TYPE_E type;
    void *slot;
} SLOT_S;

typedef int32_t (*INT_SLOT_HANLDER)(void*);
typedef int32_t (*INT_SLOT_VOID_HANLDER)(void*, void*);
typedef int32_t (*INT_SLOT_BOOL_HANLDER)(void*, bool);
typedef int32_t (*INT_SLOT_INT_HANLDER)(void*, int32_t);
typedef int32_t (*INT_SLOT_INT64_HANLDER)(void*, int64_t);
typedef int32_t (*INT_SLOT_FLOAT_HANLDER)(void*, float);
typedef int32_t (*INT_SLOT_STRING_HANLDER)(void*, char*);
typedef int32_t (*INT_SLOT_INT_INT_HANLDER)(void*, int32_t, int32_t);
typedef int32_t (*INT_SLOT_UINT32_UINT32_HANLDER)(void*, uint32_t, uint32_t);
typedef void (*VOID_SLOT_HANLDER)(void*);
typedef void (*VOID_SLOT_VOID_HANLDER)(void*, void*);
typedef void (*VOID_SLOT_BOOL_HANLDER)(void*, bool);
typedef void (*VOID_SLOT_INT_HANLDER)(void*, int32_t);
typedef void (*VOID_SLOT_INT64_HANLDER)(void*, int64_t);
typedef void (*VOID_SLOT_FLOAT_HANLDER)(void*, float);
typedef void (*VOID_SLOT_STRING_HANLDER)(void*, char*);
typedef void (*VOID_SLOT_INT_INT_HANLDER)(void*, int32_t, int32_t);
typedef void (*VOID_SLOT_UINT32_UINT32_HANLDER)(void*, uint32_t, uint32_t);

#ifdef __cplusplus
    #define SIGNAL_Emit(...) SIGNAL_EmitCpp(__VA_ARGS__)
    #define SLOT_Init(...) SLOT_InitCpp(__VA_ARGS__)
#else
    #define ARG0_SIGNAL_Emit(SIGNAL) SIGNAL_NONE_Emit(SIGNAL)
    #define ARG1_SIGNAL_Emit(SIGNAL, ARG1) \
        _Generic((ARG1), \
            bool: SIGNAL_BOOL_Emit, \
            int32_t: SIGNAL_INT_Emit, \
            int64_t: SIGNAL_INT64_Emit, \
            float: SIGNAL_FLOAT_Emit, \
            char*: SIGNAL_STRING_Emit, \
            default: SIGNAL_VOID_Emit \
        )(SIGNAL, ARG1)
    #define ARG2_SIGNAL_Emit(SIGNAL, ARG1, ARG2) \
        _Generic((ARG1), \
            int32_t: _Generic((ARG2), \
                int32_t: SIGNAL_INT_INT_Emit, \
                default: SIGNAL_INT_INT_Emit \
            ), \
            uint32_t: _Generic((ARG2), \
                uint32_t: SIGNAL_UINT32_UINT32_Emit, \
                default: SIGNAL_UINT32_UINT32_Emit \
            ) \
        )(SIGNAL, ARG1, ARG2)
    #define GET_MACRO(_1, _2, _3, NAME, ...) NAME
    #define SIGNAL_Emit(...) GET_MACRO(__VA_ARGS__, \
        ARG2_SIGNAL_Emit, \
        ARG1_SIGNAL_Emit, \
        ARG0_SIGNAL_Emit)(__VA_ARGS__)
    #define SLOT_Init(SLOT, HANLDE, HANDLER) \
        _Generic((HANDLER), \
            INT_SLOT_HANLDER: INT_SLOT_NONE_Init, \
            INT_SLOT_VOID_HANLDER: INT_SLOT_VOID_Init, \
            INT_SLOT_BOOL_HANLDER: INT_SLOT_BOOL_Init, \
            INT_SLOT_INT_HANLDER: INT_SLOT_INT_Init, \
            INT_SLOT_INT64_HANLDER: INT_SLOT_INT64_Init, \
            INT_SLOT_FLOAT_HANLDER: INT_SLOT_FLOAT_Init, \
            INT_SLOT_STRING_HANLDER: INT_SLOT_STRING_Init, \
            INT_SLOT_INT_INT_HANLDER: INT_SLOT_INT_INT_Init, \
            INT_SLOT_UINT32_UINT32_HANLDER: INT_SLOT_UINT32_UINT32_Init, \
            VOID_SLOT_HANLDER: VOID_SLOT_NONE_Init, \
            VOID_SLOT_VOID_HANLDER: VOID_SLOT_VOID_Init, \
            VOID_SLOT_BOOL_HANLDER: VOID_SLOT_BOOL_Init, \
            VOID_SLOT_INT_HANLDER: VOID_SLOT_INT_Init, \
            VOID_SLOT_INT64_HANLDER: VOID_SLOT_INT64_Init, \
            VOID_SLOT_FLOAT_HANLDER: VOID_SLOT_FLOAT_Init, \
            VOID_SLOT_STRING_HANLDER: VOID_SLOT_STRING_Init, \
            VOID_SLOT_INT_INT_HANLDER: VOID_SLOT_INT_INT_Init, \
            VOID_SLOT_UINT32_UINT32_HANLDER: VOID_SLOT_UINT32_UINT32_Init, \
            default: INT_SLOT_NONE_Init \
        )(SLOT, HANLDE, HANDLER)
#endif

#ifdef __cplusplus

void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, void *arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, bool arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, int32_t arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, int64_t arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, float arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, char *arg1);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, int32_t arg1, int32_t arg2);
void SIGNAL_EmitCpp(SIGNAL_S sysutils_signal, uint32_t arg1, uint32_t arg2);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_VOID_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_BOOL_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT64_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_FLOAT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_STRING_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT_INT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, INT_SLOT_UINT32_UINT32_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_VOID_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_BOOL_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT64_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_FLOAT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_STRING_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT_INT_HANLDER handler);
void SLOT_InitCpp(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_UINT32_UINT32_HANLDER handler);

#endif

#ifdef __cplusplus
extern "C" {
#endif

void SIGNAL_Init(SIGNAL_S *sysutils_signal);
void SIGNAL_InitByType(SIGNAL_S *sysutils_signal, SIGNAL_SLOT_TYPE_E type);
int32_t SIGNAL_Connect(SIGNAL_S sysutils_signal, SLOT_S sysutils_slot);
int32_t SIGNAL_Disconnect(SIGNAL_S sysutils_signal, SLOT_S sysutils_slot);
void SIGNAL_NONE_Emit(SIGNAL_S sysutils_signal);
void SIGNAL_VOID_Emit(SIGNAL_S sysutils_signal, void *arg1);
void SIGNAL_BOOL_Emit(SIGNAL_S sysutils_signal, bool arg1);
void SIGNAL_INT_Emit(SIGNAL_S sysutils_signal, int32_t arg1);
void SIGNAL_INT64_Emit(SIGNAL_S sysutils_signal, int64_t arg1);
void SIGNAL_FLOAT_Emit(SIGNAL_S sysutils_signal, float arg1);
void SIGNAL_STRING_Emit(SIGNAL_S sysutils_signal, char *arg1);
void SIGNAL_INT_INT_Emit(SIGNAL_S sysutils_signal, int32_t arg1, int32_t arg2);
void SIGNAL_UINT32_UINT32_Emit(SIGNAL_S sysutils_signal, uint32_t arg1, uint32_t arg2);
void SIGNAL_Deinit(SIGNAL_S *sysutils_signal);
void INT_SLOT_NONE_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_HANLDER handler);
void INT_SLOT_VOID_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_VOID_HANLDER handler);
void INT_SLOT_BOOL_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_BOOL_HANLDER handler);
void INT_SLOT_INT_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT_HANLDER handler);
void INT_SLOT_INT64_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT64_HANLDER handler);
void INT_SLOT_FLOAT_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_FLOAT_HANLDER handler);
void INT_SLOT_STRING_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_STRING_HANLDER handler);
void INT_SLOT_INT_INT_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_INT_INT_HANLDER handler);
void INT_SLOT_UINT32_UINT32_Init(SLOT_S *sysutils_slot, void *handle, INT_SLOT_UINT32_UINT32_HANLDER handler);
void VOID_SLOT_NONE_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_HANLDER handler);
void VOID_SLOT_BOOL_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_BOOL_HANLDER handler);
void VOID_SLOT_VOID_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_VOID_HANLDER handler);
void VOID_SLOT_INT_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT_HANLDER handler);
void VOID_SLOT_INT64_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT64_HANLDER handler);
void VOID_SLOT_FLOAT_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_FLOAT_HANLDER handler);
void VOID_SLOT_STRING_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_STRING_HANLDER handler);
void VOID_SLOT_INT_INT_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_INT_INT_HANLDER handler);
void VOID_SLOT_UINT32_UINT32_Init(SLOT_S *sysutils_slot, void *handle, VOID_SLOT_UINT32_UINT32_HANLDER handler);
void SLOT_Deinit(SLOT_S *sysutils_slot);

#ifdef __cplusplus
}
#endif
