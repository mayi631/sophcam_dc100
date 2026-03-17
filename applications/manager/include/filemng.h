#ifndef _FILEMNG_H_
#define _FILEMNG_H_

#include <stdint.h>
#include "appcomm.h"
#include "dtcf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FILEMNG_PATH_MAX_LEN (DTCF_PATH_MAX_LEN)
#define FILEMNG_SUFFIX_MAX_LEN (16)
#define FILEMNG_STREAM_CNT (2)

typedef enum _FILEMNG_DIR_E {
    FILEMNG_DIR_NORMAL = 0,
    FILEMNG_DIR_NORMAL_S,
    FILEMNG_DIR_PARK,
    FILEMNG_DIR_PARK_S,
    FILEMNG_DIR_EMR,
    FILEMNG_DIR_EMR_S,
    FILEMNG_DIR_PHOTO,
    FILEMNG_DIR_BUTT
} FILEMNG_DIR_E;

/**disk state */
typedef enum cviFILEMNG_STORAGE_STATE_E {
	FILEMNG_STORAGE_STATE_NOT_AVAILABLE = 0,
	FILEMNG_STORAGE_STATE_AVAILABLE = 1,
	FILEMNG_STORAGE_STATE_SCAN_COMPLETED = 2,
	FILEMNG_STORAGE_STATE_BUTT
} FILEMNG_STORAGE_STATE_E;

typedef enum cviEVENT_FILEMNG_E {
    EVENT_FILEMNG_SCAN_COMPLETED = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 0),
    EVENT_FILEMNG_SCAN_FAIL = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 1),
    EVENT_FILEMNG_SPACE_FULL = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 2),
    EVENT_FILEMNG_SPACE_ENOUGH = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 3),
    EVENT_FILEMNG_REPAIR_BEGIN = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 4),
    EVENT_FILEMNG_REPAIR_END = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 5),
    EVENT_FILEMNG_REPAIR_FAILED = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 6),
    EVENT_FILEMNG_UNIDENTIFICATION = APPCOMM_EVENT_ID(APP_MOD_FILEMNG, 7),
    EVENT_FILEMNG_BUTT
} FILEMNG_EVENT_E;

typedef struct cviFILEMNG_COMM_PARAM {
	char storage_mount_point[FILEMNG_PATH_MAX_LEN];
	char root_path[FILEMNG_PATH_MAX_LEN];
	char formated_flag[FILEMNG_PATH_MAX_LEN];
	int32_t reserved_space_percent;
	int32_t deleted_space_percent; /* use_percent * deleted_space_percent when space full */
	int32_t recloop_en;
	int32_t stream_cnt;
} FILEMNG_COMM_PARAM_S;

typedef struct cviFILEMNG_PREALLOC_PARAM {
	int32_t en;
	char suffix[FILEMNG_SUFFIX_MAX_LEN];
} FILEMNG_PREALLOC_PARAM_S;

typedef struct cviFILEMNG_DIR {
	char dirname[FILEMNG_DIR_BUTT][FILEMNG_PATH_MAX_LEN];
	char filesuffix[FILEMNG_DIR_BUTT][FILEMNG_SUFFIX_MAX_LEN];
	int32_t use_percent[FILEMNG_DIR_BUTT];
	int32_t align_size[FILEMNG_DIR_BUTT];
	int32_t prealloc_sizeMB[FILEMNG_DIR_BUTT];
} FILEMNG_DIR_S;

typedef struct cviFILEMNG_PARAM {
	FILEMNG_COMM_PARAM_S comm_param;
	FILEMNG_PREALLOC_PARAM_S prealloc_param;
	FILEMNG_DIR_S dir_param[FILEMNG_STREAM_CNT];
} FILEMNG_PARAM_S;


int32_t FILEMNG_Init(FILEMNG_PARAM_S *param);
int32_t FILEMNG_Deinit(void);
int32_t FILEMNG_SetStorageStatus(int32_t status);
int32_t FILEMNG_GetStorageStatus(void);
int32_t FILEMNG_GetStorageFormated(void);
int32_t FILEMNG_CreateStorageFormatedFlag(void);
int32_t FILEMNG_AddFile(int32_t inx, char *filename);
int32_t FILEMNG_DelFile(int32_t inx, char *filename);
int32_t FILEMNG_MoveFile(int32_t inx, FILEMNG_DIR_E dstdir, char *filename);
int32_t FILEMNG_GenerateFileName(int32_t inx, FILEMNG_DIR_E dir, const char *format, char *filename, int32_t len);
uint64_t FILEMNG_GetDirFileSize(int32_t inx, FILEMNG_DIR_E dir);
uint32_t FILEMNG_GetDirFileCnt(int32_t inx, FILEMNG_DIR_E dir);
int32_t FILEMNG_GetDirName(int32_t inx, FILEMNG_DIR_E dir, char *name, uint32_t len);
int32_t FILEMNG_RenameFile(int32_t inx, char *oldfilename, char *newfilename);
int32_t FILEMNG_GetFileNameByFileInx(int32_t inx, FILEMNG_DIR_E dir, uint32_t fileinx, char (*filename)[FILEMNG_PATH_MAX_LEN], uint32_t cnt);
int32_t FILEMNG_GetStreamInxByFileName(const char *filename);
int32_t FILEMNG_GetFilePath(int32_t inx, FILEMNG_DIR_E dir, char *filepath);

#ifdef __cplusplus
}
#endif

#endif
