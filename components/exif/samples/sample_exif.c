#include "exif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    EXIF_FILE_INFO_S exifFileInfo;
    char exifData[1164];
    uint32_t totalLen;

    // 初始化 EXIF 文件信息
    EXIF_MakeExifParam(&exifFileInfo);

    // 生成 EXIF 数据
    if (EXIF_MakeExifFile(exifData, &totalLen, &exifFileInfo) != 0) {
        printf("Failed to make EXIF file\n");
        return -1;
    }

    // 从缓冲区生成新的 JPEG 文件，并插入 EXIF 数据
    if (EXIF_MakeNewSatJpgFromBuf("input.jpg", "output.jpg", exifData, totalLen) != 0) {
        printf("Failed to create new JPEG file with EXIF data\n");
        return -1;
    }

    printf("Successfully created new JPEG file with EXIF data\n");
    return 0;
}
