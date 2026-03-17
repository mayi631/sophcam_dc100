/**
 * File:   thm_image.h
 * Author: AWTK Develop Team
 * Brief:  image
 *
 * Copyright (c) 2018 - 2021  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-02-03 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_THM_IMAGE_H
#define TK_THM_IMAGE_H

#include "base/widget.h"
#include "base/image_base.h"

BEGIN_C_DECLS
typedef ret_t (*thm_image_prepare_image_t)(uint32_t index, bitmap_t* image);

typedef struct _thm_image_t {
  image_base_t image_base;

  /**
   * @property {image_draw_type_t} draw_type
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 图片的绘制方式(仅在没有旋转和缩放时生效)。
   */
  image_draw_type_t draw_type;

  /*private*/
  uint32_t fileindex;
  thm_image_prepare_image_t prepare_image;
  bool_t pressed;
} thm_image_t;

/**
 * @method thm_image_create
 * 创建image对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t* thm_image_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method thm_image_set_draw_type
 * 设置图片的绘制方式。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget image对象。
 * @param {image_draw_type_t}  draw_type 绘制方式(仅在没有旋转和缩放时生效)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t thm_image_set_draw_type(widget_t* widget, image_draw_type_t draw_type);

/**
 * @method thm_image_set_prepare_image
 * 设置prepare_image回调函数。
 *
 * prepare_image回调函数在每次绘制之前被调用，用于准备下一帧要显示的图片。
 * 比如获取摄像头的预览图片，将其设置到image参数中。
 *
 * 注意：在回调函数中，只能修改图片的内容，不用修改图片的大小和格式，如果不匹配请先转换。
 *
 * @param {widget_t*} widget thm_image对象。
 * @param {thm_image_prepare_image_t} prepare_image 准备图片的回调函数。
 * @param {void*} prepare_image_ctx prepare_image回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t thm_image_set_prepare_image(widget_t* widget, thm_image_prepare_image_t prepare_image,
                                      uint32_t index);

/**
 * @method image_cast
 * 转换为image对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget image对象。
 *
 * @return {widget_t*} image对象。
 */
widget_t* thm_image_cast(widget_t* widget);

#define THM_IMAGE(widget) ((thm_image_t*)(thm_image_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(thm_image);

END_C_DECLS

#endif /*TK_IMAGE_H*/
