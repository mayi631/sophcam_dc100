#ifndef __STG__
#define __STG__
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STG_PATH_LEN_MAX (256)

#define PROC_FILE_PATH "/proc/cvi/cvi_info"
#define PROC_MOUNT_PATH "/proc/mounts"
#define STG_DEVINFO_PROC_LINE_LEN (64)
#define STG_MOUNTINFO_PROC_LINE_LEN (256)

#define STG_DEVINFO_PORT_INVALID "invalid"
#define STG_DEVINFO_STATE_UNPLUGGED "unplugged_disconnected"
#define STG_DEVINFO_STATE_UNPLUGGED_CONNECTED "unplugged"
#define STG_DEVINFO_STATE_CONNECTING "plugged_disconnected"
#define STG_DEVINFO_STATE_CONNECTED "plugged_connected"
#define STG_DEVINFO_VALUE_RESERVED "Reserved"
#define STG_DEVINFO_VALUE_PREFIX ":"

#define STG_MOUNTINFO_STATE_READ_ONLY "ro,"
#define STG_MOUNTINFO_STATE_READ_WRITE_AVAILABLE "rw,"

#define STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT (3)
#define STG_FSTOOL_FSTYPE_ID_OFFSET_VFAT (82)
#define STG_FSTOOL_FSTYPE_ID_LENGTH (8)
#define STG_FSTOOL_FSTYPE_ID_EXFAT "EXFAT   "
#define STG_FSTOOL_FSTYPE_ID_VFAT "FAT32   "
#define STG_FSTOOL_FSTYPE_EXFAT "exfat"
#define STG_FSTOOL_FSTYPE_VFAT "vfat"

typedef struct {
	uint64_t u64ClusterSize;   /**<DEV partition cluster size(unit bytes) */
	uint64_t u64TotalSize;     /**<DEV partition total space size(unit bytes) */
	uint64_t u64AvailableSize; /**<DEV partition free space size(unit bytes) */
	uint64_t u64UsedSize;      /**<DEV partition used space size(unit bytes) */
} STG_FS_INFO_S;

typedef enum tagSTG_DEV_STATE_E {
	STG_DEV_STATE_UNPLUGGED = 0x00,
	STG_DEV_STATE_CONNECTING,
	STG_DEV_STATE_CONNECTED,
	STG_DEV_STATE_IDLE
} STG_DEV_STATE_E;


typedef enum tagSTG_FS_TYPE_E {
	STG_FS_TYPE_FAT32 = 0x00,
	STG_FS_TYPE_EXFAT,
	STG_FS_TYPE_UNKNOWN
} STG_FS_TYPE_E;

typedef enum STORAGE_STATE_E {
	STG_STATE_DEV_UNPLUGGED = 0x00, /**<device already plugout */
	STG_STATE_DEV_CONNECTING,       /**<device connecting */
	STG_STATE_DEV_ERROR,            /**<sd card error */
	STG_STATE_FS_CHECKING,          /**<device doing fscheck */
	STG_STATE_FS_CHECK_FAILED,      /**<device fscheck failed */
	STG_STATE_FS_EXCEPTION,         /**<device file system exception */
	STG_STATE_MOUNTED,              /**<device mounted */
	STG_STATE_UNMOUNTED,            /**<device unmounted */
	STG_STATE_MOUNT_FAILED,         /**<device mount fail */
	STG_STATE_FORMATING,         	/**<device foramating */
	STG_STATE_FORMAT_SUCCESSED,     /**<device foramat successed */
	STG_STATE_FORMAT_FAILED,     	/**<device foramat failed */
	STG_STATE_READ_ONLY,     	    /**<device read only */
	STG_STATE_IDLE                  /**init state */
} STG_STATE_E;

typedef struct {
	STG_DEV_STATE_E devState;
	STG_STATE_E stgState;
	char fsType[STG_PATH_LEN_MAX];
	STG_FS_TYPE_E fsType_e;
	char workMode[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevPort[STG_DEVINFO_PROC_LINE_LEN];
	char speedClass[STG_DEVINFO_PROC_LINE_LEN];
	char speedGrade[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevType[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevPath[STG_PATH_LEN_MAX];
	char aszMntPath[STG_PATH_LEN_MAX];
	char aszErrCnt[STG_PATH_LEN_MAX];
} STG_DEVINFO_S;

typedef enum cviSTG_TRANSMISSION_SPEED_E {
    STG_TRANSMISSION_SPEED_1_4M = 0x00, /**1-4   MB/s */
    STG_TRANSMISSION_SPEED_4_10M,       /**4-10  MB/s */
    STG_TRANSMISSION_SPEED_10_30M,      /**10-30 MB/s */
    STG_TRANSMISSION_SPEED_30_50M,      /**30-50 MB/s */
    STG_TRANSMISSION_SPEED_50_100M,     /**50-100MB/s */
    STG_TRANSMISSION_SPEED_EXCEED_100M, /**100MB/s and faster */
    STG_TRANSMISSION_SPEED_BUTT         /***others**/
} STG_TRANSMISSION_SPEED_E;

typedef struct cviSTG_DEV_INFO_S {
    char aszDevType[STG_PATH_LEN_MAX];   /**device type,such as SD or MMC */
    char aszWorkMode[STG_PATH_LEN_MAX];  /**device work mode */
    STG_TRANSMISSION_SPEED_E enTranSpeed;   /**device transmission rate info */
	uint32_t szErrCnt;
} STG_DEV_INFO_S;

typedef void* STG_HANDLE_T;

#define STG_DEVINFO_HS "HS"
#define STG_DEVINFO_UHS "UHS"

#define STG_DEVINFO_UHS_SDR12 "SDR12"
#define STG_DEVINFO_UHS_SDR25 "SDR25"
#define STG_DEVINFO_UHS_SDR50 "SDR50"
#define STG_DEVINFO_UHS_DDR50 "DDR50"
#define STG_DEVINFO_UHS_SDR104 "SDR104"
#define STG_DEVINFO_UHS_DDR200 "DDR200"

#define STG_DEVINFO_SPEED_GRADE_LOW "Less than 10MB/sec(0h)"
#define STG_DEVINFO_SPEED_GRADE_HIGH "10MB/sec and above(1h)"

#define STG_DEVINFO_SPEED_CLASS_10 "Class 10"
#define STG_DEVINFO_SPEED_CLASS_6 "Class 6"
#define STG_DEVINFO_SPEED_CLASS_4 "Class 4"
#define STG_DEVINFO_SPEED_CLASS_2 "Class 2"

#define STG_DEVINFO_DEV_TYPE_LINE_NO (1)
#define STG_DEVINFO_WORK_MODE_LINE_NO (2)
#define STG_DEVINFO_SPEED_CLASS_LINE_NO (3)
#define STG_DEVINFO_SPEED_GRADE_LINE_NO (4)
#define STG_DEVINFO_ERROR_COUNT_LINE_NO (5)
#define STG_DEVINFO_PROC_LINES (5)


int32_t STG_DeInit(STG_HANDLE_T stg_hdl);
int32_t STG_Init(STG_DEVINFO_S *stgdev, STG_HANDLE_T *stg_hdl);
int32_t STG_GetFsInfo(STG_HANDLE_T stg_hdl, STG_FS_INFO_S *fsinfo);
int32_t STG_GetSDInfo(STG_HANDLE_T stg_hdl);
int32_t STG_GetFsType(STG_HANDLE_T stg_hdl);
int32_t STG_GetDevState(STG_HANDLE_T stg_hdl, STG_DEV_STATE_E *state);
int32_t STG_Umount(STG_HANDLE_T stg_hdl);
int32_t STG_Mount(STG_HANDLE_T stg_hdl);
int32_t STG_Format(STG_HANDLE_T stg_hdl);
int32_t STG_FormatWithLabel(STG_HANDLE_T stg_hdl, char *labelname);
int32_t STG_ReadProc(STG_DEVINFO_S *pstDevInfo);
int32_t STG_GetInfo(STG_HANDLE_T stg_hdl, STG_DEV_INFO_S *pInfo);
int32_t STG_RepairFAT32(STG_HANDLE_T stg_hdl);
int32_t STG_DetectPartition(STG_HANDLE_T stg_hdl);
int32_t STG_TestPartition(STG_HANDLE_T stg_hdl);
bool STG_CheckIsReadOnly(STG_DEVINFO_S *pstDevInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
