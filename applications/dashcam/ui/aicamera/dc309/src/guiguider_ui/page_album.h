
#ifndef __PAGE_ALBUM_H_
#define __PAGE_ALBUM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    ALBUM_AI = 0,     // AI处理
    ALBUM_DELETEONE,  //删除当前
    ALBUM_DELETEALL,  //删除全部
    ALBUM_LOCKONE,    //锁定当前
    ALBUM_LOCKALL,    //锁定全部
    ALBUM_UNLOCKONE,  //解锁当前
    ALBUM_UNLOCKALL,  //解锁全部
    ALBUM_PLAYBACK,   //回放
    ALBUM_ROTATE,     //旋转
    ALBUM_INFO,       //文件信息
    ALBUM_MODECHANGE, //模式转换

} AlbumMenu_Event_e;

extern lv_obj_t *obj_Aibum_s;
void Home_Album(lv_ui_t *ui);
void Home_Album_from_Pic(lv_ui_t *ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
