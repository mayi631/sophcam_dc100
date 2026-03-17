#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>

#include "cvi_log.h"
#include "osal.h"
#include "dtcf.h"
#include "storagemng.h"
#include "file_recover.h"
#include "sysutils_eventhub.h"
#include "filemng.h"

#include "record_service.h"

#define AICAM_FILENAME_PREFIX "DSCF"
#define PHOTO_ALBUM_MOVIE_PATH "/mnt/sd/DCIM/MOVIE/"
#define PHOTO_ALBUM_IMAGE_PATH "/mnt/sd/DCIM/PHOTO/"

typedef struct FILEMNG_CTX {
    FILEMNG_DIR_S param;
    uint64_t available_size_KB[FILEMNG_DIR_BUTT];
    OSAL_MUTEX_HANDLE_S mutex;
    void* dtcf_hdl;
} FILEMNG_CTX_S;

typedef struct FILEMNG {
	FILEMNG_COMM_PARAM_S comm_param;
	FILEMNG_PREALLOC_PARAM_S prealloc_param;
	FILEMNG_CTX_S ctx[FILEMNG_STREAM_CNT];
	OSAL_MUTEX_HANDLE_S mutex;
	OSAL_TASK_HANDLE_S storage_check;
	int32_t storage_check_exit;
	int32_t storage_status;
} FILEMNG_S;

static FILEMNG_S *g_filemng = NULL;
static int32_t g_photo_file_index = 0;
static int32_t g_movie_file_index = 0;

#define FILEMNG_CHECK(ptr)           \
	do {                             \
		if (ptr == NULL) {           \
			CVI_LOGE("ptr is NULL"); \
			return -1;               \
		}                            \
	} while (0)

static int32_t filemng_CreateDir(FILEMNG_CTX_S *ctx)
{
	char dirname[FILEMNG_DIR_BUTT][FILEMNG_PATH_MAX_LEN];
	int32_t cnt = 0;
	for (int32_t i = 0; i < FILEMNG_DIR_BUTT; i++) {
		if (strlen(ctx->param.dirname[i]) > 0) {
			strncpy(dirname[cnt], ctx->param.dirname[i], FILEMNG_PATH_MAX_LEN - 1);
			CVI_LOGI("create dir %s\n", dirname[cnt]);
			cnt++;
		}
	}
	CVI_LOGI("create dir cnt %d\n", cnt);
	if (cnt > 0) {
		DTCF_CreateDir(ctx->dtcf_hdl, dirname, cnt, DTCF_DIR_MODE);
	}
	return 0;
}

static int32_t filemng_Selector(const struct dirent *d)
{
	char *tag = strrchr(d->d_name, '.');
	if (tag) {
		for (int32_t i = 0; i < RECORD_SERVICE_FILE_TYPE_MAX; i++) {
			if (strcasecmp(tag, FILE_TYPE_SUFFIX[i]) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

static int32_t filemng_SelectorPrealloc(const struct dirent *d)
{
	if (strncasecmp(d->d_name, g_filemng->prealloc_param.suffix, strlen(g_filemng->prealloc_param.suffix)) == 0) {
		return 1;
	}
	return 0;
}

static int32_t filemng_CompareFileName(const struct dirent **a, const struct dirent **b)
{
	return alphasort(a, b);
}

static int32_t filemng_ReadLine(const char *file, char *line, int32_t max_len)
{
	char temp[FILEMNG_PATH_MAX_LEN] = {0};
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		CVI_LOGE("fopen %s failed %s", file, strerror(errno));
		return -1;
	}
	while (1) {
		if (fgets(temp, FILEMNG_PATH_MAX_LEN, fp) == NULL) {
			break;
		}

		if (temp[strlen(temp) - 1] == '\n') {
			temp[strlen(temp) - 1] = '\0';
		}

		if (temp[strlen(temp) - 1] == '\r') {
			temp[strlen(temp) - 1] = '\0';
		}

		if (DTCF_is_ValidChar(temp) == 0) {
			continue;
		}

		strncpy(line, temp, max_len - 1);
		break;
	}
	fclose(fp);
	return strlen(line);
}

static int32_t filemng_DeleteLine(const char *file, const char *line)
{
#define TEMP_FILE_NAME "/tmp/.sdconfig"
	char buffer[FILEMNG_PATH_MAX_LEN];

	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		CVI_LOGE("Failed to open file %s", file);
		return -1;
	}
	FILE *temp_fp = fopen(TEMP_FILE_NAME, "w");

	if (temp_fp == NULL) {
		CVI_LOGE("Failed to open file %s", TEMP_FILE_NAME);
		fclose(fp);
		return -1;
	}

	// Read lines from the original file and write them to the temporary file,
	// except for the line that needs to be deleted.
	int32_t n = 0;
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		n = strlen(buffer);
		if (buffer[n - 1] == '\n') {
			n--;
		}

		if (buffer[n - 1] == '\r') {
			n--;
		}
		if (n > 0 && strncmp(buffer, line, n) != 0) {
			fputs(buffer, temp_fp);
		}
	}

	fclose(fp);
	fclose(temp_fp);

	// Remove the original file
	if (remove(file) != 0) {
		CVI_LOGE("remove failed %s", file);
		return -1;
	}

	// Rename the temporary file to the original file name
	if (rename(TEMP_FILE_NAME, file) != 0) {
		// CVI_LOGE("rename failed %s", file);
		char cmd[FILEMNG_PATH_MAX_LEN * 2] = {0};
		snprintf(cmd, sizeof(cmd), "mv %s %s", TEMP_FILE_NAME, file);
		OSAL_FS_System(cmd);
	}

	OSAL_FS_Async();
	return 0;
}

static int32_t filemng_WriteLine(const char *file, const char *line)
{
	int32_t fd = open(file, O_WRONLY);
	if (fd < 0 || strlen(line) <= 0) {
		CVI_LOGE("open %s failed", file);
		return -1;
	}
	lseek(fd, 0, SEEK_END);
	write(fd, line, strlen(line));
	close(fd);
	OSAL_FS_Async();
	return 0;
}

static int32_t filemng_FileRecover(const char *file, int32_t prealloc)
{
	int32_t ret = 0;
	if (file == NULL || strlen(file) <= 0) {
		return -1;
	}

	if (access(file, F_OK) != 0) {
		return -1;
	}

	char line[FILEMNG_PATH_MAX_LEN] = {0};
	FILE_RECOVER_HANDLE_T hdl = NULL;
	ret = FILE_RECOVER_Create(&hdl);
	if (ret != 0) {
		CVI_LOGE("create recover handle failed");
		return -1;
	}

	while (1) {
		memset(line, 0x0, FILEMNG_PATH_MAX_LEN);
		ret = filemng_ReadLine(file, line, FILEMNG_PATH_MAX_LEN);
		if (ret > 0) {
			char *tag = strrchr(line, '.');
			if (tag == NULL) {
				continue;
			}

			int32_t suffixFound = 0;
			for (int32_t i = 0; i < RECORD_SERVICE_FILE_TYPE_MAX; i++) {
				if (i != RECORD_SERVICE_FILE_TYPE_MP4 && i != RECORD_SERVICE_FILE_TYPE_MOV) {
					continue;
				}
				if (strcasecmp(tag, FILE_TYPE_SUFFIX[i]) == 0) {
					suffixFound = 1;
				}
			}
			if (suffixFound == 1) {
				// FILEMNG_EVENT_REPAIR_BEGIN
				EVENT_S stEvent = {0};
				stEvent.topic = EVENT_FILEMNG_REPAIR_BEGIN;
				EVENTHUB_Publish(&stEvent);
				CVI_LOGI("recover filename %s %d", line, prealloc);
				FILE_RECOVER_PreallocateState(hdl, prealloc);
				ret = FILE_RECOVER_Open(hdl, line);
				if (ret == 0) {
					ret = FILE_RECOVER_Recover(hdl, line, "", true);
				}

				if (ret != 0) {
					// FILEMNG_EVENT_REPAIR_FAILED
					stEvent.topic = EVENT_FILEMNG_REPAIR_FAILED;
					EVENTHUB_Publish(&stEvent);
					// int32_t inx = FILEMNG_GetStreamInxByFileName(line);
					// FILEMNG_DelFile(inx, line);
					// remove(line);
					// OSAL_FS_Async();
				} else {
					stEvent.topic = EVENT_FILEMNG_REPAIR_END;
					EVENTHUB_Publish(&stEvent);
				}
				FILE_RECOVER_Destroy(&hdl);
				// FILEMNG_EVENT_REPAIR_END
			}
			filemng_DeleteLine(file, line);
		} else {
			break;
		}
	}
	return 0;
}
static uint32_t filemng_StorageCheckSpace(FILEMNG_S *filemng)
{
	OSAL_MUTEX_Lock(g_filemng->mutex);

	//循环使能，会自动删除文件
	if (filemng->comm_param.recloop_en) {
		for (int32_t i = 0; i < filemng->comm_param.stream_cnt; i++) {
			for (int32_t j = FILEMNG_DIR_NORMAL; j < FILEMNG_DIR_BUTT; j++) {
				if (strlen(filemng->ctx[i].param.dirname[j]) <= 0) {
					continue;
				}
				uint64_t dir_size = 0;
				DTCF_GetDirFileSize(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], &dir_size);
				if (dir_size > filemng->ctx[i].available_size_KB[j]) {
					CVI_LOGI("dir %s available_size_KB %llu size %llu\n", filemng->ctx[i].param.dirname[j], filemng->ctx[i].available_size_KB[j], dir_size);
					uint64_t need_delete_size = 0;
					need_delete_size = filemng->ctx[i].available_size_KB[j] * filemng->comm_param.deleted_space_percent / 100;
					if (filemng->prealloc_param.en) {
						need_delete_size = dir_size - filemng->ctx[i].available_size_KB[j];
					}
					CVI_LOGI("need delete size %llu\n", need_delete_size);
					// DTCF_ShowFileList(filemng->ctx[i].dtcf_hdl, __func__);
					while (1) {
						char filename[FILEMNG_PATH_MAX_LEN] = {0};
						if (filemng->prealloc_param.en) {
							DTCF_GetPreallocFileIdleFilename(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], filename, FILEMNG_PATH_MAX_LEN - 1);
							if (strlen(filename) > 0) {
								DTCF_DelPreallocFile(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], filename);
							}
						}
						if (strlen(filename) <= 0) {
							DTCF_GetFileNameByInx(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], &filename, 1, 0);
							if (strlen(filename) > 0) {
								DTCF_DelFile(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], filename);
							}
						}
						if (strlen(filename) <= 0) {
							CVI_LOGI("no file in dir %s\n", filemng->ctx[i].param.dirname[j]);
							OSAL_MUTEX_Unlock(g_filemng->mutex);
							return -1;
						} else {
							CVI_LOGI("delete file %s\n", filename);
							char fullpath[FILEMNG_PATH_MAX_LEN] = {0};
							snprintf(fullpath, FILEMNG_PATH_MAX_LEN, "%s/%s/%s/%s",
									filemng->comm_param.storage_mount_point, filemng->comm_param.root_path,
									filemng->ctx[i].param.dirname[j], filename);
							if (remove(fullpath) != 0) {
								CVI_LOGE("remove file %s failed: %s\n", fullpath, strerror(errno));
							}
						}
						dir_size = 0;
						DTCF_GetDirFileSize(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], &dir_size);
						if (filemng->ctx[i].available_size_KB[j] - dir_size > need_delete_size) {
							break;
						}
					}
				}
				if (filemng->prealloc_param.en) {
					if (dir_size + filemng->ctx[i].param.prealloc_sizeMB[j] * 1024 * 2 < filemng->ctx[i].available_size_KB[j]) {
						char filename[FILEMNG_PATH_MAX_LEN] = {0};
						char index[FILEMNG_SUFFIX_MAX_LEN] = {0};
						DTCF_GetPreallocFileIdleInx(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], index);
						snprintf(filename, FILEMNG_PATH_MAX_LEN, "%s%s", filemng->prealloc_param.suffix, index);
						DTCF_CreatePreallocFile(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], filename, filemng->ctx[i].param.prealloc_sizeMB[j] * 1024);
					}
				}
			}
		}
		OSAL_FS_Async();
	} else {
		STG_FS_INFO_S pstInfo;
		int32_t ret = STORAGEMNG_GetFSInfo(&pstInfo);
		if (ret != 0) {
			CVI_LOGE("Get FS info failed");
			OSAL_MUTEX_Unlock(g_filemng->mutex);
			return 0;
		}
		uint64_t storage_spaceKB = pstInfo.u64TotalSize >> 10;
		uint64_t u64AvailableSize_KB = pstInfo.u64AvailableSize >> 10;
		uint64_t u64Reserve_KB = storage_spaceKB * filemng->comm_param.reserved_space_percent / 100;
		if (u64AvailableSize_KB < u64Reserve_KB) {
			CVI_LOGE("Storage space is not enough, available size %llu KB, reserved size %llu KB", u64AvailableSize_KB, u64Reserve_KB);
			OSAL_MUTEX_Unlock(g_filemng->mutex);
			return -1;
		}
	}

	OSAL_MUTEX_Unlock(g_filemng->mutex);
	return 0;
}

/*
Get the file size. If the size exceeds the specified size, do not process it. If it is less than
the specified size, align it according to the specified size
 */
static int32_t filemng_AlignRecordFileSize(FILEMNG_S *filemng, int32_t stream_inx, int32_t dir_inx)
{
	if (!filemng->prealloc_param.en) {
		return 0;
	}

	if (strlen(filemng->ctx[stream_inx].param.dirname[dir_inx]) <= 0 ||
		filemng->ctx[stream_inx].param.prealloc_sizeMB[dir_inx] <= 0) {
		return 0;
	}

	uint32_t file_count = 0;
	uint32_t offset = 0;
	uint32_t batch_size = 50;
	DTCF_GetDirFileCnt(filemng->ctx[stream_inx].dtcf_hdl, filemng->ctx[stream_inx].param.dirname[dir_inx], &file_count);
	char(*filenames)[FILEMNG_PATH_MAX_LEN] = NULL;
	const uint64_t target_size = filemng->ctx[stream_inx].param.prealloc_sizeMB[dir_inx] * 1024;
	uint64_t filesize = 0;

	while (file_count > 0) {
		uint32_t cnts = ((file_count > batch_size) ? batch_size : file_count);
		filenames = malloc(cnts * FILEMNG_PATH_MAX_LEN);
		if (!filenames) {
			CVI_LOGE("Memory allocation failed\n");
			return -1;
		}

		int32_t ret = DTCF_GetFileNameByInx(filemng->ctx[stream_inx].dtcf_hdl,
											filemng->ctx[stream_inx].param.dirname[dir_inx], filenames, cnts, offset);
		if (ret <= 0) {
			free(filenames);
			break;
		}

		offset += cnts;
		file_count -= cnts;

		for (uint32_t k = 0; k < file_count; k++) {
			char fullpath[FILEMNG_PATH_MAX_LEN];
			snprintf(fullpath, sizeof(fullpath), "%s/%s/%s/%s",
					 filemng->comm_param.storage_mount_point,
					 filemng->comm_param.root_path,
					 filemng->ctx[stream_inx].param.dirname[dir_inx],
					 filenames[k]);

			OSAL_FS_Du(fullpath, &filesize);

			if (filesize >= target_size) {
				continue; // 文件大小已足够
			}
			DTCF_ReallocPreallocFile(filemng->ctx[stream_inx].dtcf_hdl, filemng->ctx[stream_inx].param.dirname[dir_inx], filenames[k], target_size);
		}

		free(filenames);
	}

        return 0;
}

static int32_t filemng_ParseNumber_FromFilename(const char* fullpath, const char* format)
{
    int max_num = -1;
    struct dirent* ent;
    int32_t file_index = 0;
    const char* real_path_photo = PHOTO_ALBUM_IMAGE_PATH;
    const char* real_path_movie = PHOTO_ALBUM_MOVIE_PATH;
    DIR* file_dir = opendir(fullpath);

    if (!file_dir) {
        CVI_LOGE("Failed to open directory '%s'\n", fullpath);
        return 1;
    }

    while ((ent = readdir(file_dir)) != NULL) {
        char* file_name = ent->d_name;
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
            continue;

        size_t len = strlen(file_name);
        size_t pre_len = strlen(AICAM_FILENAME_PREFIX);
        size_t suf_len = strlen(format);

        if (len < pre_len + suf_len + 1)
            continue;

        if (strncmp(file_name, AICAM_FILENAME_PREFIX, pre_len) != 0)
            continue;
        if (strcmp(file_name + len - suf_len, format) != 0)
            continue;

        size_t digits_start = pre_len;
        size_t digits_end = len - suf_len;
        if (digits_end <= digits_start)
            continue;

        int num = 0;
        int has_digit = 0;
        for (size_t i = digits_start; i < digits_end; ++i) {
            if (!isdigit((unsigned char)file_name[i])) {
                // 遇到非数字字符停止，只取第一组数字
                break;
            }
            has_digit = 1;
            int d = file_name[i] - '0';
            num = num * 10 + d;
        }

        if (has_digit && num > max_num)
            max_num = num;
    }

    file_index = (max_num >= 0) ? (max_num + 1) : 1;
    closedir(file_dir);

    if (strcmp(fullpath, real_path_photo) == 0) {
        g_photo_file_index = file_index;
    } else if (strcmp(fullpath, real_path_movie) == 0) {
        g_movie_file_index = file_index;
    }
    CVI_LOGI("Parse index from:'%s';PHOTO:%d;MOVIE:%d;\n", fullpath, g_photo_file_index, g_movie_file_index);

    return 0;
}

static void filemng_InitFileIndex(void)
{
    CVI_LOGI("Init File Index\n");
    char* real_path_photo = PHOTO_ALBUM_IMAGE_PATH;
    char* real_path_movie = PHOTO_ALBUM_MOVIE_PATH;

    filemng_ParseNumber_FromFilename(real_path_photo, ".jpg");
    filemng_ParseNumber_FromFilename(real_path_movie, ".mov");
}

static void filemng_StorageCheckTask(void *arg)
{
	FILEMNG_S *filemng = (FILEMNG_S *)arg;
	STG_FS_INFO_S pstInfo;
	int32_t storage_status = FILEMNG_STORAGE_STATE_NOT_AVAILABLE;
	DTCF_PARAM_S dtcf_param;
	int32_t ret = 0;
	while (filemng->storage_check_exit == 0) {
		if (storage_status != filemng->storage_status) {
			if (filemng->storage_status == FILEMNG_STORAGE_STATE_AVAILABLE && storage_status == FILEMNG_STORAGE_STATE_NOT_AVAILABLE) {
				memset(&dtcf_param, 0x0, sizeof(DTCF_PARAM_S));
				STORAGEMNG_GetFSInfo(&pstInfo);
				uint64_t storage_spaceKB = pstInfo.u64TotalSize / 1024;
				storage_spaceKB -= (storage_spaceKB * filemng->comm_param.reserved_space_percent / 100);

				snprintf(dtcf_param.main_dir, FILEMNG_PATH_MAX_LEN, "%s/%s", filemng->comm_param.storage_mount_point, filemng->comm_param.root_path);
				dtcf_param.file_selector_cb = filemng_Selector;
				if (filemng->prealloc_param.en) {
					dtcf_param.file_selector_prealloc_cb = filemng_SelectorPrealloc;
				}
				dtcf_param.file_compare_cb = filemng_CompareFileName;

				uint64_t dir_totalsize_KB = 0;
				for (int32_t i = 0; i < filemng->comm_param.stream_cnt; i++) {
					ret = DTCF_Init(&filemng->ctx[i].dtcf_hdl, &dtcf_param);
					if (ret != 0) {
						CVI_LOGE("DTCF_Init failed");
						break;
					}

					filemng_CreateDir(&filemng->ctx[i]);

					ret = DTCF_Scan(filemng->ctx[i].dtcf_hdl);
					if (ret != 0) {
						CVI_LOGE("DTCF_Scan failed");
						break;
					}

					for (int32_t j = FILEMNG_DIR_NORMAL; j < FILEMNG_DIR_BUTT; j++) {
						uint64_t dir_size = 0;
						if (strlen(filemng->ctx[i].param.dirname[j]) <= 0) {
							continue;
						}
						DTCF_GetDirFileSize(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], &dir_size);
						dir_totalsize_KB += dir_size;
					}
				}
				if (pstInfo.u64UsedSize / 1024 > dir_totalsize_KB) {
					storage_spaceKB -= (pstInfo.u64UsedSize / 1024 - dir_totalsize_KB);
				}
				CVI_LOGI("storage_spaceKB %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", storage_spaceKB, dir_totalsize_KB, pstInfo.u64ClusterSize / 1024, pstInfo.u64UsedSize / 1024, pstInfo.u64AvailableSize / 1024);

				for (int32_t i = 0; i < filemng->comm_param.stream_cnt; i++) {

					// filemng_AlignRecordFileSize(filemng, i, j)

					for (int32_t j = FILEMNG_DIR_NORMAL; j < FILEMNG_DIR_BUTT; j++) {
						if (strlen(filemng->ctx[i].param.dirname[j]) <= 0) {
							continue;
						}

						uint64_t dir_size = 0;
						uint64_t delete_size = filemng->ctx[i].available_size_KB[j] * filemng->comm_param.deleted_space_percent / 100;
						filemng->ctx[i].available_size_KB[j] = storage_spaceKB * filemng->ctx[i].param.use_percent[j] / 100;
						DTCF_GetDirFileSize(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], &dir_size);

						CVI_LOGI("%s available_size [%" PRIu64 " KB] used [%" PRIu64 " KB]\n", filemng->ctx[i].param.dirname[j], filemng->ctx[i].available_size_KB[j], dir_size);
						if (filemng->prealloc_param.en) {
							if (dir_size + delete_size < filemng->ctx[i].available_size_KB[j]) {
								int32_t remain = (int32_t)(filemng->ctx[i].available_size_KB[j] - dir_size - delete_size) / 1024;
								while (remain > filemng->ctx[i].param.prealloc_sizeMB[j]) {
									char filename[FILEMNG_PATH_MAX_LEN] = {0};
									char index[FILEMNG_SUFFIX_MAX_LEN] = {0};
									DTCF_GetPreallocFileIdleInx(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], index);
									snprintf(filename, FILEMNG_PATH_MAX_LEN, "%s%s", filemng->prealloc_param.suffix, index);
									ret = DTCF_CreatePreallocFile(filemng->ctx[i].dtcf_hdl, filemng->ctx[i].param.dirname[j], filename, filemng->ctx[i].param.prealloc_sizeMB[j] * 1024);
									if (ret != 0) {
										CVI_LOGW("DTCF_CreatePreallocFile fail, ret = %d\n", ret);
										break;
									}
									remain -= filemng->ctx[i].param.prealloc_sizeMB[j];
								}
							}
						}
					}
				}

				char temp[FILEMNG_PATH_MAX_LEN] = {0};
				snprintf(temp, FILEMNG_PATH_MAX_LEN, "%s/%s", filemng->comm_param.storage_mount_point, filemng->comm_param.formated_flag);
				filemng_FileRecover(temp, filemng->prealloc_param.en);

				OSAL_MUTEX_Lock(g_filemng->mutex);
				filemng_InitFileIndex();
				OSAL_MUTEX_Unlock(g_filemng->mutex);
				if (ret != 0) {
					// FILEMNG_EVENT_SCAN_FAIL
					filemng->storage_status = FILEMNG_STORAGE_STATE_NOT_AVAILABLE;
					EVENT_S stEvent = {0};
					stEvent.topic = EVENT_FILEMNG_SCAN_FAIL;
					EVENTHUB_Publish(&stEvent);
				} else {
					filemng->storage_status = FILEMNG_STORAGE_STATE_SCAN_COMPLETED;
					EVENT_S stEvent = {0};
					stEvent.topic = EVENT_FILEMNG_SCAN_COMPLETED;
					EVENTHUB_Publish(&stEvent);
					// FILEMNG_EVENT_SCAN_COMPLETED
					CVI_LOGI("storage scan completed\n");
				}
			} else if (filemng->storage_status == FILEMNG_STORAGE_STATE_NOT_AVAILABLE) {
				for (int32_t i = 0; i < filemng->comm_param.stream_cnt; i++) {
					DTCF_Deinit(filemng->ctx[i].dtcf_hdl);
					filemng->ctx[i].dtcf_hdl = NULL;
				}
			}
			OSAL_FS_Async();
			storage_status = filemng->storage_status;
		}

		if (storage_status == FILEMNG_STORAGE_STATE_SCAN_COMPLETED) {
			ret = filemng_StorageCheckSpace(filemng);
			if (ret != 0) {
				// FILEMNG_EVENT_SPACE_FULL
				EVENT_S stEvent = {0};
				stEvent.topic = EVENT_FILEMNG_SPACE_FULL;
				EVENTHUB_Publish(&stEvent);
			}
		}
		OSAL_TASK_Sleep(500 * 1000);
	}
}

static int32_t filemng_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_SCAN_COMPLETED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_SCAN_FAIL);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_SPACE_FULL);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_SPACE_ENOUGH);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_REPAIR_BEGIN);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_REPAIR_END);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_REPAIR_FAILED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_FILEMNG_UNIDENTIFICATION);
	return s32Ret;
}

int32_t FILEMNG_Init(FILEMNG_PARAM_S *param)
{
	CVI_LOGI("FILEMNG_Init Compile time: %s %s\n", __DATE__, __TIME__);
	if (g_filemng != NULL) {
		CVI_LOGI("CVI_FILEMNG already init\n");
		return 0;
	}

	if (param->comm_param.stream_cnt > FILEMNG_STREAM_CNT) {
		CVI_LOGE("stream_cnt is too large %d\n", param->comm_param.stream_cnt);
		return -1;
	}

	if (param->comm_param.reserved_space_percent > 100 || param->comm_param.reserved_space_percent < 0) {
		CVI_LOGE("reserved_space_percent is too large %d\n", param->comm_param.reserved_space_percent);
		return -1;
	}

	g_filemng = (FILEMNG_S *)malloc(sizeof(FILEMNG_S));
	memset(g_filemng, 0, sizeof(FILEMNG_S));

	g_filemng->comm_param = param->comm_param;
	g_filemng->prealloc_param = param->prealloc_param;

	int32_t total_percent = 0;
	for (int32_t i = 0; i < param->comm_param.stream_cnt; i++) {
		g_filemng->ctx[i].param = param->dir_param[i];
		g_filemng->ctx[i].dtcf_hdl = NULL;
		OSAL_MUTEX_Create(NULL, &g_filemng->ctx[i].mutex);
		for (int32_t j = FILEMNG_DIR_NORMAL; j < FILEMNG_DIR_BUTT; j++) {
			if (param->dir_param[i].dirname[j][0] != '\0') {
				total_percent += param->dir_param[i].use_percent[j];
			}
		}
	}

	if (total_percent > 100) {
		CVI_LOGE("total percent is %d, should be less than 100\n", total_percent);
		for (int32_t i = 0; i < param->comm_param.stream_cnt; i++) {
			OSAL_MUTEX_Destroy(g_filemng->ctx[i].mutex);
		}
		free(g_filemng);
		g_filemng = NULL;
		return -1;
	}

	char path[FILEMNG_PATH_MAX_LEN] = {0};
	snprintf(path, FILEMNG_PATH_MAX_LEN, "%s/%s", param->comm_param.storage_mount_point, param->comm_param.root_path);
	OSAL_FS_Mkdir(path, DTCF_DIR_MODE);

	g_filemng->storage_status = FILEMNG_STORAGE_STATE_NOT_AVAILABLE;
	OSAL_MUTEX_Create(NULL, &g_filemng->mutex);
	g_filemng->storage_check_exit = 0;
	OSAL_TASK_ATTR_S ta;
	ta.name = "StorageCheckTask";
	ta.entry = filemng_StorageCheckTask;
	ta.param = (void *)g_filemng;
	ta.priority = OSAL_TASK_PRI_NORMAL;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	OSAL_TASK_Create(&ta, &g_filemng->storage_check);

	filemng_RegisterEvent();
	return 0;
}

int32_t FILEMNG_Deinit(void)
{
	FILEMNG_CHECK(g_filemng);
	g_filemng->storage_check_exit = 1;
	OSAL_TASK_Join(g_filemng->storage_check);
	OSAL_TASK_Destroy(&g_filemng->storage_check);
	g_filemng->storage_status = FILEMNG_STORAGE_STATE_NOT_AVAILABLE;
	for (int32_t i = 0; i < g_filemng->comm_param.stream_cnt; i++) {
		OSAL_MUTEX_Lock(g_filemng->ctx[i].mutex);
		DTCF_Deinit(g_filemng->ctx[i].dtcf_hdl);
		g_filemng->ctx[i].dtcf_hdl = NULL;
		OSAL_MUTEX_Unlock(g_filemng->ctx[i].mutex);
		OSAL_MUTEX_Destroy(g_filemng->ctx[i].mutex);
	}
	OSAL_MUTEX_Destroy(g_filemng->mutex);
	free(g_filemng);
	g_filemng = NULL;
	return 0;
}

int32_t FILEMNG_SetStorageStatus(int32_t status)
{
	FILEMNG_CHECK(g_filemng);
	g_filemng->storage_status = status;
	return 0;
}

int32_t FILEMNG_GetStorageStatus(void)
{
	FILEMNG_CHECK(g_filemng);
	return g_filemng->storage_status;
}

int32_t FILEMNG_GetStorageFormated(void)
{
	char formated_flag[FILEMNG_PATH_MAX_LEN] = {0};
	snprintf(formated_flag, FILEMNG_PATH_MAX_LEN, "%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.formated_flag);
	if (access(formated_flag, F_OK) == 0) {
		CVI_LOGI("Storage is Formated !!!!!\n");
		return 1;
	}
	CVI_LOGI("Storage is Not Formated !!!!!\n");
	return 0;
}

int32_t FILEMNG_CreateStorageFormatedFlag(void)
{
	char formated_flag[FILEMNG_PATH_MAX_LEN] = {0};
	snprintf(formated_flag, FILEMNG_PATH_MAX_LEN, "%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.formated_flag);
	OSAL_FS_Touch(formated_flag);
	OSAL_FS_SetHiddenAttribute(formated_flag, 1);
	return 0;
}

/*
fullpath : /mnt/sd/ cardv/ movie0/ 20241011205800_00_F.mp4
dir : movie0
file : 20241011205800_00_F.mp4
*/
static int32_t filemng_Parsefilename(const char *fullpath, char *dir, char *file)
{
	const char *lastSlash = strrchr(fullpath, '/');
	if (lastSlash == NULL) {
		return -1;
	}

	const char *fileName = lastSlash + 1;
	size_t dirLength = lastSlash - fullpath;

	if (dirLength > 0) {
		strncpy(dir, fullpath, dirLength);
		dir[dirLength] = '\0';
		const char *lastDirSlash = strrchr(dir, '/');
		if (lastDirSlash) {
			snprintf(dir, dirLength + 1, "%s", lastDirSlash + 1);
		}
	} else {
		dir[0] = '\0';
	}

	strncpy(file, fileName, strlen(fileName) + 1);

	return 0;
}

int32_t FILEMNG_AddFile(int32_t inx, char *filename)
{
	FILEMNG_CHECK(g_filemng);
	FILEMNG_CHECK(g_filemng->ctx[inx].dtcf_hdl);
	char dir[FILEMNG_PATH_MAX_LEN] = {0};
	char file[FILEMNG_PATH_MAX_LEN] = {0};
	int32_t ret = filemng_Parsefilename(filename, dir, file);
	if (ret != 0) {
		CVI_LOGE("FILEMNG_Parsefilename failed");
		return -1;
	}

	FILEMNG_DIR_E srcdir = FILEMNG_DIR_BUTT;
	for (int32_t i = 0; i < FILEMNG_DIR_BUTT; i++) {
		if (strcmp(g_filemng->ctx[inx].param.dirname[i], dir) == 0) {
			srcdir = i;
		}
	}

	if (srcdir == FILEMNG_DIR_BUTT) {
		CVI_LOGE("FILEMNG_AddFile dir %s not found", dir);
		return -1;
	}

	OSAL_MUTEX_Lock(g_filemng->mutex);
	char temp[FILEMNG_PATH_MAX_LEN] = {0};
	snprintf(temp, FILEMNG_PATH_MAX_LEN, "%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.formated_flag);
	filemng_DeleteLine(temp, filename);
	OSAL_MUTEX_Unlock(g_filemng->mutex);
	if (g_filemng->prealloc_param.en) {
		CVI_LOGI("###### inx %d dir %s file %s filename %s", inx, dir, file, filename);
		DTCF_UpdateSizePreallocFile(g_filemng->ctx[inx].dtcf_hdl, dir, file, g_filemng->ctx[inx].param.align_size[srcdir]);
	} else {
		ret = DTCF_AddFile(g_filemng->ctx[inx].dtcf_hdl, dir, file);
		if (ret != 0) {
			CVI_LOGE("DTCF_AddFile failed %d %s %s", inx, dir, file);
			return -1;
		}
	}
	return 0;
}

int32_t FILEMNG_DelFile(int32_t inx, char *filename)
{
	FILEMNG_CHECK(g_filemng);
	FILEMNG_CHECK(g_filemng->ctx[inx].dtcf_hdl);
	char dir[FILEMNG_PATH_MAX_LEN] = {0};
	char file[FILEMNG_PATH_MAX_LEN] = {0};
	int32_t ret = filemng_Parsefilename(filename, dir, file);
	if (ret != 0) {
		CVI_LOGE("FILEMNG_Parsefilename failed");
		return -1;
	}

	if (g_filemng->prealloc_param.en) {
		char prealloc_file[FILEMNG_PATH_MAX_LEN] = {0};
		char index[FILEMNG_SUFFIX_MAX_LEN] = {0};
		DTCF_GetPreallocFileIdleInx(g_filemng->ctx[inx].dtcf_hdl, dir, index);
		snprintf(prealloc_file, FILEMNG_PATH_MAX_LEN, "%s%s", g_filemng->prealloc_param.suffix, index);
		ret = DTCF_RenamePreallocFile(g_filemng->ctx[inx].dtcf_hdl, dir, prealloc_file, file, 1);
	} else {
		ret = DTCF_DelFile(g_filemng->ctx[inx].dtcf_hdl, dir, file);
		if (ret == 0) {
			remove(filename);
		}
	}

	OSAL_FS_Async();
	return ret;
}

/* Renaming or locking existing files requires changing the directory,
   such as entering the file list to rename files or locking them from
   regular video files to emergency video files
*/
int32_t FILEMNG_RenameFile(int32_t inx, char *oldfilename, char *newfilename)
{
	FILEMNG_CHECK(g_filemng);
	FILEMNG_CHECK(g_filemng->ctx[inx].dtcf_hdl);
	char dir[2][FILEMNG_PATH_MAX_LEN] = {0};
	char file[2][FILEMNG_PATH_MAX_LEN] = {0};
	int32_t ret = filemng_Parsefilename(oldfilename, dir[0], file[0]);
	ret |= filemng_Parsefilename(newfilename, dir[1], file[1]);
	if (ret != 0) {
		CVI_LOGE("FILEMNG_Parsefilename failed");
		return -1;
	}
	ret = DTCF_RenameFile(g_filemng->ctx[inx].dtcf_hdl, dir[0], file[0], dir[1], file[1]);
	if (ret == 0) {
		rename(oldfilename, newfilename);
	}
	return ret;
}

/* Move files that have just been out of file and have not yet been added to DTCF.
 For example, if a regular video file triggers an emergency event during the
 recording process and becomes an emergency video file, it needs to be renamed
 and moved to the emergency video directory after being out of file
*/
int32_t FILEMNG_MoveFile(int32_t inx, FILEMNG_DIR_E dstdir, char *filename)
{
	FILEMNG_CHECK(g_filemng);
	FILEMNG_CHECK(g_filemng->ctx[inx].dtcf_hdl);
	char dir[FILEMNG_PATH_MAX_LEN] = {0};
	char srcfile[FILEMNG_PATH_MAX_LEN] = {0};
	char dstfile[FILEMNG_PATH_MAX_LEN] = {0};
	int32_t file_index = 0;
	int32_t ret = filemng_Parsefilename(filename, dir, srcfile);
	if (ret != 0) {
		CVI_LOGE("FILEMNG_Parsefilename failed");
		return -1;
	}

	FILEMNG_DIR_E srcdir = FILEMNG_DIR_BUTT;
	for (int32_t i = 0; i < FILEMNG_DIR_BUTT; i++) {
		if (strcmp(g_filemng->ctx[inx].param.dirname[i], dir) == 0) {
			srcdir = i;
		}
	}

	// for example: /mnt/sd/cardv/movie0/20241011205800_00_N.mp4 to /mnt/sd/cardv/emr0/20241011205800_01_E.mp4
	// srcfile == 20241011205800_00_N.mp4
TRYAGAIN:
	memset(dstfile, 0x0, FILEMNG_PATH_MAX_LEN);
	// 1. /mnt/sd/cardv/emr0
	// snprintf(dstfile, FILEMNG_PATH_MAX_LEN, "%s/%s/%s/",
	// 		 g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.root_path,
	// 		 g_filemng->ctx[inx].param.dirname[dstdir]);

	// extsuffix == _N.mp4
	char *extsuffix = strstr(srcfile, g_filemng->ctx[inx].param.filesuffix[srcdir]);
	// suffix == .mp4
	char *suffix = strrchr(srcfile, '.');
	if (extsuffix) {
		// 2. 20241011205800
		strncpy(dstfile, srcfile, strlen(srcfile) - strlen(extsuffix) - strlen("_00"));
	}

	// 3. 20241011205800_00
	snprintf(dstfile, FILEMNG_PATH_MAX_LEN, "_%02d", file_index);

	// 4. 20241011205800_00_E
	strncat(dstfile, g_filemng->ctx[inx].param.filesuffix[dstdir], strlen(g_filemng->ctx[inx].param.filesuffix[dstdir]));
	// 5. 20241011205800_00_E.mp4
	strncat(dstfile, suffix, strlen(suffix));

	if (access(dstfile, F_OK) == 0) {
		file_index++;
		if (file_index >= 99) {
			CVI_LOGW("file name overflow\n");
			return -1;
		}
		goto TRYAGAIN;
	}

	OSAL_MUTEX_Lock(g_filemng->mutex);
	char temp[FILEMNG_PATH_MAX_LEN] = {0};
	snprintf(temp, FILEMNG_PATH_MAX_LEN, "%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.formated_flag);
	filemng_DeleteLine(temp, filename);
	OSAL_MUTEX_Unlock(g_filemng->mutex);

	ret = DTCF_AddFile(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dstdir], dstfile);
	if (ret != 0) {
		CVI_LOGE("DTCF_AddFile failed %d %d %s", inx, dstdir, dstfile);
		return -1;
	}
	return ret;
}

uint64_t FILEMNG_GetDirFileSize(int32_t inx, FILEMNG_DIR_E dir)
{
	if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
		return -1;
	}

	if (strlen(g_filemng->ctx[inx].param.dirname[dir]) <= 0) {
		return -1;
	}
	uint64_t size = 0;
	DTCF_GetDirFileSize(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], &size);
	return size;
}

uint32_t FILEMNG_GetDirFileCnt(int32_t inx, FILEMNG_DIR_E dir)
{
	if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
		return -1;
	}

	if (strlen(g_filemng->ctx[inx].param.dirname[dir]) <= 0) {
		return -1;
	}
	uint32_t cnt = 0;
	DTCF_GetDirFileCnt(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], &cnt);
	return cnt;
}

int32_t FILEMNG_GetDirName(int32_t inx, FILEMNG_DIR_E dir, char *name, uint32_t len)
{
	if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
		return -1;
	}

	if (strlen(g_filemng->ctx[inx].param.dirname[dir]) > 0) {
		strncpy(name, g_filemng->ctx[inx].param.dirname[dir], len);
	}
	return 0;
}

int32_t FILEMNG_GenerateFileName(int32_t inx, FILEMNG_DIR_E dir, const char* format, char* filename, int32_t len)
{
    FILEMNG_CHECK(g_filemng);
    FILEMNG_CHECK(g_filemng->ctx[inx].param.dirname[dir]);
    if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
        return -1;
    }

    if (strlen(g_filemng->ctx[inx].param.dirname[dir]) <= 0) {
        return -1;
    }

    memset(filename, 0x0, len);
    char file[FILEMNG_PATH_MAX_LEN] = { 0 };
    char fullpath[FILEMNG_PATH_MAX_LEN] = { 0 };
    int32_t ret = 0;
    memset(fullpath, 0x0, FILEMNG_PATH_MAX_LEN);
    snprintf(fullpath, FILEMNG_PATH_MAX_LEN, "%s/%s/%s/",
        g_filemng->comm_param.storage_mount_point,
        g_filemng->comm_param.root_path,
        g_filemng->ctx[inx].param.dirname[dir]);

    char* real_path_photo = PHOTO_ALBUM_IMAGE_PATH;
    char* real_path_movie = PHOTO_ALBUM_MOVIE_PATH;
    int32_t* file_index = &g_photo_file_index;
    if (strcmp(fullpath, real_path_photo) == 0) {
        file_index = &g_photo_file_index;
    } else if (strcmp(fullpath, real_path_movie) == 0) {
        file_index = &g_movie_file_index;
    }

    if (*file_index == 0) {
        CVI_LOGE("file_index=0\n");
        ret = filemng_ParseNumber_FromFilename(fullpath, format);
        if (ret != 0) {
            CVI_LOGE("Failed to parse number from filename\n");
            return -1;
        }
    }

TRY_AGIAN:
    snprintf(file, FILEMNG_PATH_MAX_LEN, "DSCF%05d%s%s",
        *file_index,
        g_filemng->ctx[inx].param.filesuffix[dir],
        format);
    snprintf(fullpath + strlen(fullpath), FILEMNG_PATH_MAX_LEN - strlen(fullpath), "%s", file);
    if (access(fullpath, F_OK) == 0) {
        (*file_index)++;
        fullpath[strlen(fullpath) - strlen(file)] = '\0';
        OSAL_TASK_Sleep(100 * 1000);
        goto TRY_AGIAN;
    }

    if (g_filemng->prealloc_param.en) {
        char prealloc_filename[FILEMNG_PATH_MAX_LEN] = { 0 };
        ret = DTCF_GetPreallocFileIdleFilename(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], prealloc_filename, FILEMNG_PATH_MAX_LEN - 1);
        if (ret != 0) {
            char firstFileName[FILEMNG_PATH_MAX_LEN] = { 0 };
            ret = DTCF_GetFileNameByInx(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], &firstFileName, 1, 0);
            if (ret == 0 && strlen(firstFileName) > 0) {
                CVI_LOGI("DTCF_RenameFile file %s", firstFileName);
                DTCF_RenameFile(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], firstFileName, g_filemng->ctx[inx].param.dirname[dir], file);
            } else {
                CVI_LOGE("Failed to get the first file name from the list\n");
                return -1;
            }
        } else {
            CVI_LOGI("DTCF_RenamePreallocFile file %s", prealloc_filename);
            DTCF_RenamePreallocFile(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], prealloc_filename, file, 0);
        }
    }
    CVI_LOGI("filemng_WriteLine file %s", file);
    OSAL_MUTEX_Lock(g_filemng->mutex);
    char temp[FILEMNG_PATH_MAX_LEN] = { 0 };
    snprintf(temp, FILEMNG_PATH_MAX_LEN, "%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.formated_flag);
    strncpy(filename, fullpath, len);
    fullpath[strlen(fullpath)] = '\n';
    filemng_WriteLine(temp, fullpath);
    OSAL_MUTEX_Unlock(g_filemng->mutex);
    OSAL_FS_Async();
    CVI_LOGI("OSAL_FS_Async file %s", file);
    return 0;
}

int32_t FILEMNG_GetFileNameByFileInx(int32_t inx, FILEMNG_DIR_E dir, uint32_t fileinx, char (*filename)[FILEMNG_PATH_MAX_LEN], uint32_t cnt)
{
	if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
		return -1;
	}

	if (strlen(g_filemng->ctx[inx].param.dirname[dir]) <= 0) {
		return -1;
	}
	return DTCF_GetFileNameByInx(g_filemng->ctx[inx].dtcf_hdl, g_filemng->ctx[inx].param.dirname[dir], filename, cnt, fileinx);
}

int32_t FILEMNG_GetStreamInxByFileName(const char *filename)
{
	FILEMNG_CHECK(g_filemng);

	char dirname[FILEMNG_PATH_MAX_LEN] = {0};
	char srcfile[FILEMNG_PATH_MAX_LEN] = {0};
	int32_t ret = filemng_Parsefilename(filename, dirname, srcfile);
	if (ret != 0) {
		CVI_LOGE("FILEMNG_Parsefilename failed");
		return -1;
	}
	int32_t streamInx = -1;
	for (int32_t i = 0; i < FILEMNG_STREAM_CNT; i++) {
		for (int32_t j = 0; j < FILEMNG_DIR_BUTT; j++) {
			if (strcmp(g_filemng->ctx[i].param.dirname[j], dirname) == 0) {
				streamInx = i;
				break;
			}
		}
	}
	return streamInx;
}

int32_t FILEMNG_GetFilePath(int32_t inx, FILEMNG_DIR_E dir, char *filepath)
{
	if (inx >= FILEMNG_STREAM_CNT || dir >= FILEMNG_DIR_BUTT) {
		return -1;
	}

	if (strlen(g_filemng->ctx[inx].param.dirname[dir]) <= 0) {
		return -1;
	}
	snprintf(filepath, FILEMNG_PATH_MAX_LEN, "%s/%s/%s", g_filemng->comm_param.storage_mount_point, g_filemng->comm_param.root_path, g_filemng->ctx[inx].param.dirname[dir]);

	return 0;
}
