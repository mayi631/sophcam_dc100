#ifndef __PARAM_PRINT_H__
#define __PARAM_PRINT_H__
#include "param.h"

const char* bool_to_str(bool value);
void print_param_cam_cfg(const PARAM_CAM_CFG *cam);
void print_param_cfg(const PARAM_CFG_S *cfg);
void print_param_filemng(const PARAM_FILEMNG_S *filemng);
void print_param_devmng(const PARAM_DEVMNG_S *devmng);
void print_param_workmode(const PARAM_WORK_MODE_S *workmode);
void print_param_media_comm(const PARAM_MEDIA_COMM_S *mediacomm);
void print_param_menu(const PARAM_MENU_S *menu);

#endif /* __PARAM_PRINT_H__ */
