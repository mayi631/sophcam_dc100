#ifndef __EXIF_H__
#define __EXIF_H__
#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EXIF_FILE_SIZE 28800
#define MAX_JPG_THUMBNAIL_WIDTH 320
#define MAX_JPG_THUMBNAIL_HEIGHT 240
#define MAX_FILE_THUMB_SIZE (MAX_JPG_THUMBNAIL_WIDTH * MAX_JPG_THUMBNAIL_HEIGHT)

typedef struct tagExifFileInfo
{
    char Make[32];
    char Model[32];
    char Version[32];
    char DateTime[32];
    char CopyRight[32];
    char description[32];
    uint32_t Height;
    uint32_t Width;
    uint32_t Orientation;
    uint32_t ColorSpace;
    uint32_t Process;
    uint32_t Flash;
    uint32_t FocalLengthNum;
    uint32_t FocalLengthDen;
    uint32_t ExposureTimeNum;
    uint32_t ExposureTimeDen;
    uint32_t FNumberNum;
    uint32_t FNumberDen;
    uint32_t ApertureFNumber;
    int32_t SubjectDistanceNum;
    int32_t SubjectDistanceDen;
    uint32_t CCDWidth;
    int32_t ExposureBiasNum;
    int32_t ExposureBiasDen;
    int32_t WhiteBalance;
    uint32_t MeteringMode;
    int32_t ExposureProgram;
    uint32_t ISOSpeedRatings[2];
    uint32_t FocalPlaneXResolutionNum;
    uint32_t FocalPlaneXResolutionDen;
    uint32_t FocalPlaneYResolutionNum;
    uint32_t FocalPlaneYResolutionDen;
    uint32_t FocalPlaneResolutionUnit;
    uint32_t XResolutionNum;
    uint32_t XResolutionDen;
    uint32_t YResolutionNum;
    uint32_t YResolutionDen;
    uint32_t RUnit;
    int32_t BrightnessNum;
    int32_t BrightnessDen;
    char UserComments[150];
    char GpsLatitudeRef;
    unsigned char GpsLatitude[20];
    char GpsLongitudeRef;
    unsigned char GpsLongitude[20];
    unsigned char GpsAltitude[10];
    bool parame;
} EXIF_FILE_INFO_S;

int32_t EXIF_MakeExifFile(char *ExifOut, uint32_t *totalLen, EXIF_FILE_INFO_S *exifFileInfo);
void EXIF_MakeExifParam(EXIF_FILE_INFO_S *exifFileInfo);
int32_t EXIF_MakeNewSatJpgFromBuf(const char *src_file, const char *dest_file, char *buf, int32_t size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
