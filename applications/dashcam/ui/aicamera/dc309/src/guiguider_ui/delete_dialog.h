/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef __DELETE_DIALOG_H_
#define __DELETE_DIALOG_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sure_delete_callback_t)(void);

// 快捷删除确认
void create_simple_delete_dialog(const char *file_name);
// 同时调用---获取所有的照片名或视频名
void get_all_filenames(char ***filenames, int *total_files);
void clean_all_malloc(char **filenames, int total_files);
void sure_delete_register_callback(sure_delete_callback_t callback);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
