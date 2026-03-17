/**
 * File:   image.h
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
 * 2018-01-28 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "base/enums.h"
#include "thm_image/thm_image.h"
#include "tkc/utils.h"
#include "base/image_manager.h"

static ret_t thm_image_on_paint_self(widget_t* widget, canvas_t* c) {
  rect_t dst;
  bitmap_t bitmap;
  thm_image_t* thm_image = THM_IMAGE(widget);
  vgcanvas_t* vg = canvas_get_vgcanvas(c);
  return_value_if_fail(thm_image != NULL, RET_BAD_PARAMS);
  if (thm_image->prepare_image == NULL) {
    widget_paint_helper(widget, c, NULL, NULL);
    return RET_OK;
  }

  do {
    if (thm_image->prepare_image != NULL &&
        thm_image->prepare_image(thm_image->fileindex, &bitmap) == RET_OK) {
      if (vg != NULL) {
        if (image_need_transform(widget)) {
          if (thm_image->draw_type == IMAGE_DRAW_ICON || thm_image->draw_type == IMAGE_DRAW_CENTER) {
            vgcanvas_save(vg);
            image_transform(widget, c);
            if (thm_image->draw_type == IMAGE_DRAW_ICON) {
              vgcanvas_draw_icon(vg, &bitmap, 0, 0, bitmap.w, bitmap.h, 0, 0, widget->w, widget->h);
            } else {
              float_t x = (widget->w - bitmap.w) * 0.5f;
              float_t y = (widget->h - bitmap.h) * 0.5f;
              vgcanvas_draw_image(vg, &bitmap, 0, 0, bitmap.w, bitmap.h, x, y, bitmap.w, bitmap.h);
            }
            vgcanvas_restore(vg);
            break;
          } else {
            log_warn("only draw_type == icon supports transformation.\n");
          }
        }
      }

      dst = rect_init(0, 0, widget->w, widget->h);
      canvas_draw_image_ex(c, &bitmap, thm_image->draw_type, &dst);
      bitmap_destroy(&bitmap);
    }
  } while (FALSE);

  widget_paint_helper(widget, c, NULL, NULL);

  return RET_OK;
}

static ret_t thm_image_pointer_up_cleanup(widget_t* widget) {
  thm_image_t* thm_image = THM_IMAGE(widget);
  return_value_if_fail(thm_image != NULL && widget != NULL, RET_BAD_PARAMS);

  thm_image->pressed = FALSE;
  widget_ungrab(widget->parent, widget);
  widget_set_state(widget, WIDGET_STATE_NORMAL);

  return RET_OK;
}

static ret_t thm_image_on_event(widget_t* widget, event_t* e) {
  uint16_t type = e->type;
  thm_image_t* thm_image = THM_IMAGE(widget);
  return_value_if_fail(thm_image != NULL && widget != NULL, RET_BAD_PARAMS);
  switch (type) {
    case EVT_POINTER_DOWN: {
      thm_image->pressed = TRUE;
      widget_set_state(widget, WIDGET_STATE_PRESSED);
      widget_grab(widget->parent, widget);
      break;
    }
    case EVT_POINTER_DOWN_ABORT: {
      thm_image_pointer_up_cleanup(widget);
      break;
    }
    case EVT_POINTER_UP: {
      pointer_event_t evt = *(pointer_event_t*)e;
      if (thm_image->pressed && widget_is_point_in(widget, evt.x, evt.y, FALSE)) {
        evt.e = event_init(EVT_CLICK, widget);
        thm_image_pointer_up_cleanup(widget);
        widget_dispatch(widget, (event_t*)&evt);
      } else {
        thm_image_pointer_up_cleanup(widget);
      }

      break;
    }
    case EVT_POINTER_LEAVE:
      widget_set_state(widget, WIDGET_STATE_NORMAL);
      break;
    case EVT_POINTER_ENTER: {
      pointer_event_t* evt = (pointer_event_t*)e;
      if (thm_image->pressed || !evt->pressed) {
        widget_set_state(widget, WIDGET_STATE_OVER);
      }
      break;
    }
    default:
      break;
  }

  return RET_OK;
}

static const char* const s_image_properties[] = {WIDGET_PROP_IMAGE,      WIDGET_PROP_DRAW_TYPE,
                                                 WIDGET_PROP_SCALE_X,    WIDGET_PROP_SCALE_Y,
                                                 WIDGET_PROP_ANCHOR_X,   WIDGET_PROP_ANCHOR_Y,
                                                 WIDGET_PROP_ROTATION,   WIDGET_PROP_CLICKABLE,
                                                 WIDGET_PROP_SELECTABLE, NULL};

static ret_t thm_image_on_copy(widget_t* widget, widget_t* other) {
  thm_image_t* image = THM_IMAGE(widget);
  thm_image_t* image_other = THM_IMAGE(other);
  return_value_if_fail(image != NULL && image_other != NULL, RET_BAD_PARAMS);

  image_base_on_copy(widget, other);
  image->draw_type = image_other->draw_type;

  return RET_OK;
}

TK_DECL_VTABLE(thm_image) = {.size = sizeof(thm_image_t),
                         .type = WIDGET_TYPE_IMAGE,
                         .space_key_to_activate = TRUE,
                         .return_key_to_activate = TRUE,
                         .clone_properties = s_image_properties,
                         .persistent_properties = s_image_properties,
                         .parent = TK_PARENT_VTABLE(image_base),
                         .create = thm_image_create,
                         .on_copy = thm_image_on_copy,
                         .on_destroy = image_base_on_destroy,
                         .on_event = thm_image_on_event,
                         .on_paint_self = thm_image_on_paint_self,
                         .set_prop = image_base_set_prop,
                         .get_prop = image_base_get_prop};

widget_t* thm_image_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = widget_create(parent, TK_REF_VTABLE(thm_image), x, y, w, h);
  thm_image_t* image = THM_IMAGE(widget);
  return_value_if_fail(image != NULL, NULL);

  image_base_init(widget);
  image->draw_type = IMAGE_DRAW_SCALE;
  return widget;
}

ret_t thm_image_set_draw_type(widget_t* widget, image_draw_type_t draw_type) {
  thm_image_t* image = THM_IMAGE(widget);
  return_value_if_fail(image != NULL, RET_BAD_PARAMS);

  image->draw_type = draw_type;

  return widget_invalidate(widget, NULL);
}

ret_t thm_image_set_prepare_image(widget_t* widget, thm_image_prepare_image_t prepare_image,
                                      uint32_t index) {
  thm_image_t* thm_image = THM_IMAGE(widget);
  return_value_if_fail(thm_image != NULL && prepare_image != NULL, RET_BAD_PARAMS);

  thm_image->prepare_image = prepare_image;
  thm_image->fileindex = index;

  return RET_OK;
}

widget_t* thm_image_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, thm_image), NULL);

  return widget;
}
