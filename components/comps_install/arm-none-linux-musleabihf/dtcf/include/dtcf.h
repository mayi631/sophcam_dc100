#ifndef _DTCF_H_
#define _DTCF_H_

#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define the maximum length of a path string
#define DTCF_PATH_MAX_LEN (256)
// Define the directory mode for creation (readable and writable by owner, readable by group and others)
#define DTCF_DIR_MODE (0755)

// Typedef for file selection callback function
typedef int32_t (*FILE_SelectorCb)(const struct dirent *d);
// Typedef for file comparison callback function
typedef int32_t (*FILE_CompareCb)(const struct dirent **a, const struct dirent **b);

// Structure to hold parameters for DTCF initialization
typedef struct DTCF_PARAM {
	char main_dir[DTCF_PATH_MAX_LEN];	   // Main directory path
	FILE_SelectorCb file_selector_cb;		   // Callback for selecting files
	FILE_SelectorCb file_selector_prealloc_cb; // Callback for selecting preallocated files
	FILE_CompareCb file_compare_cb;			   // Callback for comparing files
} DTCF_PARAM_S;

// Function to show the list of files
void DTCF_ShowFileList(void *hdl, const char *func);

// Function to check if a character is valid
int32_t DTCF_is_ValidChar(char *str);

// Function to initialize the DTCF module
int32_t DTCF_Init(void **hdl, DTCF_PARAM_S *param);

// Function to deinitialize the DTCF module
int32_t DTCF_Deinit(void *hdl);

// Function to create directories
int32_t DTCF_CreateDir(void *hdl, const char dirs[][DTCF_PATH_MAX_LEN], int32_t cnt, mode_t mode);

// Function to scan the directory
int32_t DTCF_Scan(void *hdl);

// Function to add a file to the directory
int32_t DTCF_AddFile(void *hdl, char *dir, char *file);

// Function to delete a file from the directory
int32_t DTCF_DelFile(void *hdl, char *dir, char *file);

// Function to rename a file
int32_t DTCF_RenameFile(void *hdl, char *olddir, char *oldfile, char *newdir, char *newfile);

// Function to get the total size of files in a directory
int32_t DTCF_GetDirFileSize(void *hdl, char *dir, uint64_t *size);

// Function to get the number of files in a directory
int32_t DTCF_GetDirFileCnt(void *hdl, char *dir, uint32_t *cnt);

// Function to get file names by index
int32_t DTCF_GetFileNameByInx(void *hdl, char *dir, char file[][DTCF_PATH_MAX_LEN], uint32_t cnt, uint32_t start);

// Function to get the idle index of a preallocated file
int32_t DTCF_GetPreallocFileIdleInx(void *hdl, char *dir, char *strinx);

// Function to get the idle filename of a preallocated file
int32_t DTCF_GetPreallocFileIdleFilename(void *hdl, char *dir, char *file, int32_t len);

// Function to rename a preallocated file
int32_t DTCF_RenamePreallocFile(void *hdl, char *dir, char *prealloc, char *file, int32_t flag);

// Function to create a preallocated file
int32_t DTCF_CreatePreallocFile(void *hdl, char *dir, char *file, uint32_t sizeKB);

// Function to delete a preallocated file
int32_t DTCF_DelPreallocFile(void *hdl, char *dir, char *file);

// Function to reallocate a preallocated file
int32_t DTCF_ReallocPreallocFile(void *hdl, char *dir, char *file, uint64_t sizeKB);

int32_t DTCF_UpdateSizePreallocFile(void *hdl, char *dir, char *file, uint64_t alignSizeKB);

#ifdef __cplusplus
}
#endif

#endif