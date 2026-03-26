#ifndef __THUMBNAIL_EXTRACTOR_H__
#define __THUMBNAIL_EXTRACTOR_H__

#include <stdint.h>
#include "demuxer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NS_EXPORT

/*
 * To work around http://code.google.com/p/android/issues/detail?id=23203
 * we don't link with the crt objects. In some configurations, this means
 * a lack of the __dso_handle symbol because it is defined there, and
 * depending on the android platform and ndk versions used, it may or may
 * not be defined in libc.so. In the latter case, we fail to link. Defining
 * it here as weak makes us provide the symbol when it's not provided by
 * the crt objects, making the change transparent for future NDKs that
 * would fix the original problem. On older NDKs, it is not a problem
 * either because the way __dso_handle was used was already broken (and
 * the custom linker works around it).
 */
NS_EXPORT __attribute__((weak)) void *__dso_handle;

typedef void* THUMBNAIL_EXTRACTOR_HANDLE_T;
typedef DEMUXER_PACKET_S THUMBNAIL_PACKET_S;

int32_t THUMBNAIL_EXTRACTOR_Create(THUMBNAIL_EXTRACTOR_HANDLE_T *handle);
int32_t THUMBNAIL_EXTRACTOR_Destroy(THUMBNAIL_EXTRACTOR_HANDLE_T *handle);
int32_t THUMBNAIL_EXTRACTOR_GetThumbnail(THUMBNAIL_EXTRACTOR_HANDLE_T handle,
    const char *input, THUMBNAIL_PACKET_S *thumbnail);
int32_t THUMBNAIL_EXTRACTOR_GetThumbnailByType(THUMBNAIL_EXTRACTOR_HANDLE_T handle,
    const char *input, THUMBNAIL_PACKET_S *thumbnail, int thumbnail_type);
int32_t THUMBNAIL_EXTRACTOR_ClearPacket(THUMBNAIL_PACKET_S *packet);

#ifdef __cplusplus
}
#endif

#endif
