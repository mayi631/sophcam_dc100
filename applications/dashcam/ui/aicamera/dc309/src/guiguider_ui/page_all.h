
#ifndef __PAGE_ALL_H_
#define __PAGE_ALL_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "page_home.h"

#define MENU_CONT_SIZE 296
#define MENU_BTN_SIZE 58
#define MENU_FONT_SIZE 34
// 拍照设置
#include "page_photo.h"
#include "page_photomenu_setting.h"
#include "page_photomenu_res.h"
#include "page_photomenu_whitebalance.h"
#include "page_photomenu_effect.h"
#include "page_photomenu_exposure.h"
#include "page_photomenu_autofocus.h"
#include "page_photomenu_picmode.h"
#include "page_photomenu_selfietime.h"
#include "page_phoeomenu_shootmode.h"
#include "page_photomenu_picquality.h"
#include "page_phoeomenu_iso.h"
#include "page_photomenu_antishake.h"
#include "page_photomenu_facedec.h"
#include "page_photomenu_smiledec.h"
#include "page_photomenu_beauty.h"
#include "page_photomenu_aimode.h"
#include "page_photomenu_flash.h"
#include "page_photomenu_cursor.h"

// 录像设置
#include "page_vediomenu_setting.h"
#include "page_vedio.h"
#include "page_vediomenu_graphy.h"
#include "page_vediomenu_timelapse.h"
#include "page_vediomenu_sharpness.h"
#include "page_vediomenu_res.h"

// 系统设置
#include "page_sysmenu_setting.h"
#include "page_sysmenu_language.h"
#include "page_sysmenu_time.h"
#include "page_sysmenu_powerdown.h"
#include "page_sysmenu_volume.h"
#include "page_sysmenu_lightfreq.h"
#include "page_sysmenu_format.h"
#include "page_sysmenu_factory.h"
#include "page_sysmenu_wificode.h"
#include "page_sysmenu_wifilist.h"
#include "page_sysmenu_scroff.h"
#include "page_sysmenu_version.h"
#include "page_sysmenu_brightness.h"
#include "page_sysmenu_statusllight.h"
// 相册
#include "page_album.h"

// 相册图片查看
#include "page_albumPic.h"
#include "page_album_video.h"

#include "page_aitalk.h"
#include "page_ai_takephoto.h"
#include "page_shoot_translation.h"

// 翻译
#include "multilang_strings.h"
#include "custom.h"

// 共用弹窗
#include "voice_bar.h"
#include "aiprocess_pup.h"
#include "zoom_bar.h"
#include "float_effect.h"
#include "wifi_check_dialog.h"
#include "delete_dialog.h"
#include "ai_voice_style.h"
#include "batter_low_wifi_tips.h"
#include "ai_voice_custom_pop.h"

//控件style
#include "style_common.h"
#include "infrared.h"

void lvgl_page_flip_demo();

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
