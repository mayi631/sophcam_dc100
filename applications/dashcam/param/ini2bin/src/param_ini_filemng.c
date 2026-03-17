
#include "param_ini2bin.h"
#include "ini.h"
#include "param.h"
#include "param_printf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
extern PARAM_ACCESS g_ParamAccess;
/* load common start */
static int PARAM_LoadComm(const char *file, FILEMNG_COMM_PARAM_S *Comm)
{

    char storage_mount_point[FILEMNG_PATH_MAX_LEN];
	char root_path[FILEMNG_PATH_MAX_LEN];
	char formated_flag[FILEMNG_PATH_MAX_LEN];
	long int reserved_space_percent = 0;
	long int deleted_space_percent = 0; /* use_percent * deleted_space_percent when space full */
	long int stream_cnt = 0;

	INI_GetString("common", "mount_point", "", storage_mount_point, FILEMNG_PATH_MAX_LEN, file);
    INI_GetString("common", "root_path", "", root_path, FILEMNG_PATH_MAX_LEN, file);
    INI_GetString("common", "formated_flag", "", formated_flag, FILEMNG_PATH_MAX_LEN, file);
    reserved_space_percent = INI_GetLong("common", "reserved_space_percent", 0, file);
    deleted_space_percent = INI_GetLong("common", "deleted_space_percent", 0, file);
    stream_cnt = INI_GetLong("stream.dir", "cnt", 0, file);

	Comm->recloop_en = 1;
    Comm->deleted_space_percent = deleted_space_percent;
    Comm->stream_cnt = stream_cnt;
    Comm->reserved_space_percent = reserved_space_percent;
    snprintf(Comm->storage_mount_point, sizeof(Comm->storage_mount_point), "%s", storage_mount_point);
    snprintf(Comm->root_path, sizeof(Comm->root_path), "%s", root_path);
    snprintf(Comm->formated_flag, sizeof(Comm->formated_flag), "%s", formated_flag);
	return 0;
}
/* load common end */
static int PARAM_LOADPrealloc(const char *file, FILEMNG_PREALLOC_PARAM_S *Prealloc)
{
	long int preallocenable = 0;
	char suffix[FILEMNG_SUFFIX_MAX_LEN] = {0};
	preallocenable = INI_GetLong("prealloc", "enable", 0, file);
    INI_GetString("prealloc", "suffix", "", suffix, sizeof(suffix), file);
    snprintf(Prealloc->suffix, sizeof(Prealloc->suffix), "%s", suffix);
	Prealloc->en = preallocenable;
	return 0;
}

/* load dtcf end */
typedef struct cviFILEMNG_DTCF_DirName_S {
	const char *key0, *key1, *key2, *key3, *key4;
} FILEMNG_DTCF_DirName_S;
static int PARAM_LoadDirCfg(const char *file, FILEMNG_DIR_S *FileDir)
{
	FILEMNG_DTCF_DirName_S tag[FILEMNG_DIR_BUTT] = {
		{"normal_dir", "normal_filesuffix", "normal_percent", "normal_align_size", "normal_prealloc_size"},
		{"normal_s_dir", "normal_s_filesuffix", "normal_s_percent", "normal_s_align_size", "normal_s_prealloc_size"},
		{"park_dir", "park_filesuffix", "park_percent", "park_align_size", "park_prealloc_size"},
		{"park_s_dir", "park_s_filesuffix", "park_s_percent", "park_s_align_size", "park_s_prealloc_size"},
		{"emr_dir", "emr_filesuffix", "emr_percent", "emr_align_size", "emr_prealloc_size"},
		{"emr_s_dir", "emr_s_filesuffix", "emr_percent", "emr_align_size", "emr_s_prealloc_size"},
		{"photo_dir", "photo_filesuffix", "photo_percent", "photo_align_size", "photo_prealloc_size"},
	};
	char section[FILEMNG_PATH_MAX_LEN] = {0};
	long int cnt = INI_GetLong("stream.dir", "cnt", 0, file);
	for (int i = 0; i < cnt; i++) {
		snprintf(section, sizeof(section), "stream%d.dir", i);
		for (int j = 0; j < FILEMNG_DIR_BUTT; j++) {
			char dir[FILEMNG_PATH_MAX_LEN] = {0};
			char suffix[FILEMNG_SUFFIX_MAX_LEN] = {0};
			long int percent = 0;
			long int align_size = 0;
			long int prealloc_size = 0;
			INI_GetString(section, tag[j].key0, "", dir, sizeof(dir), file);
			INI_GetString(section, tag[j].key1, "", suffix, sizeof(suffix), file);
			percent = INI_GetLong(section, tag[j].key2, 0, file);
			align_size = INI_GetLong(section, tag[j].key3, 0, file);
			prealloc_size = INI_GetLong(section, tag[j].key4, 0, file);
			snprintf(FileDir[i].dirname[j], FILEMNG_PATH_MAX_LEN, "%s", dir);
			snprintf(FileDir[i].filesuffix[j], FILEMNG_SUFFIX_MAX_LEN, "%s", suffix);
			FileDir[i].use_percent[j] = percent;
			FileDir[i].align_size[j] = align_size;
			FileDir[i].prealloc_sizeMB[j] = prealloc_size;
			if (strlen(dir) > 0)
				printf("%s %s %ld %ld %ld\n", dir, suffix, percent, align_size, prealloc_size);
		}
	}
	return 0;
}
static int PARAM_LoadFilemng(const char *file, PARAM_FILEMNG_S *FileMng)
{
	int s32Ret = 0;
	s32Ret = PARAM_LoadComm(file, &FileMng->FileMng.comm_param);
	s32Ret |= PARAM_LOADPrealloc(file, &FileMng->FileMng.prealloc_param);
	s32Ret |= PARAM_LoadDirCfg(file, FileMng->FileMng.dir_param);
	return s32Ret;
}
int INI_PARAM_LoadFilemngCfg(PARAM_FILEMNG_S *FileMng)
{
	printf("\n---enter: %s\n", __func__);
	uint32_t i = 0;
	char filepath[PARAM_MODULE_NAME_LEN] = {0};
	for (i = 0; i < g_ParamAccess.module_num; i++) {
		if (strstr(g_ParamAccess.modules[i].name, "config_filemng")) {
			memset(filepath, 0, sizeof(filepath));
			snprintf(filepath, sizeof(filepath), "%s%s",
					 g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
			PARAM_LoadFilemng(filepath, FileMng);
			break;
		}
	}
	return 0;
}

void print_param_filemng(const PARAM_FILEMNG_S *filemng)
{
    if(filemng == NULL) {
        printf("[FileMng] Error: NULL pointer received.\n");
        return;
    }
    printf("[FileMng]\n");
    printf("comm_param:\n");
    printf("storage_mount_point: %s\n", filemng->FileMng.comm_param.storage_mount_point);
    printf("root_path: %s\n", filemng->FileMng.comm_param.root_path);
    printf("formated_flag: %s\n", filemng->FileMng.comm_param.formated_flag);
    printf("reserved_space_percent: %d%%\n", filemng->FileMng.comm_param.reserved_space_percent);
    printf("deleted_space_percent: %d%%\n", filemng->FileMng.comm_param.deleted_space_percent);
    printf("recloop_en: %d\n", filemng->FileMng.comm_param.recloop_en);
    printf("stream_cnt: %d\n", filemng->FileMng.comm_param.stream_cnt);
    printf("prealloc_param:\n");
    printf("en: %d\n", filemng->FileMng.prealloc_param.en);
    printf("suffix: %s\n", filemng->FileMng.prealloc_param.suffix);
    printf("dir_param:\n");
    for(int i = 0; i < FILEMNG_STREAM_CNT; i++) {
        printf("[Stream %d]\n", i);
        for(int j = 0; j < FILEMNG_DIR_BUTT; j++) {
            printf("dirname[%d]: %s\n", j, filemng->FileMng.dir_param[i].dirname[j]);
        }
        for(int j = 0; j < FILEMNG_DIR_BUTT; j++) {
            printf("filesuffix[%d]: %s\n", j, filemng->FileMng.dir_param[i].filesuffix[j]);
        }
        printf("use_percent: ");
        for(int j = 0; j < FILEMNG_DIR_BUTT; j++) {
            printf("%d%% ", filemng->FileMng.dir_param[i].use_percent[j]);
        }
        printf("\n");
        printf("align_size:   ");
        for(int j = 0; j < FILEMNG_DIR_BUTT; j++) {
            printf("%d ", filemng->FileMng.dir_param[i].align_size[j]);
        }
        printf("\n");
        printf("prealloc_sizeMB: ");
        for(int j = 0; j < FILEMNG_DIR_BUTT; j++) {
            printf("%dMB ", filemng->FileMng.dir_param[i].prealloc_sizeMB[j]);
        }
        printf("\n");
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
