#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
// #include "page_all.h"
#include "config.h"
#include "gui_guider.h"
#include "custom.h"

lv_ui_t g_ui;

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", FB_DEV_NAME);
    lv_display_t *disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
    lv_display_set_resolution(disp, H_RES, V_RES);

    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, TOUCH_PANEL_EVENT_PATH);
    lv_indev_set_display(touch, disp);

    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width  = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "640");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "480");

    lv_group_set_default(lv_group_create());

    lv_display_t *disp = lv_sdl_window_create(width, height);

    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    lv_obj_t *cursor_obj;
    cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
    lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
    lv_indev_set_cursor(mouse, cursor_obj);           /*Connect the image  object to the driver*/

    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());

    // return disp;
}
#else
#error Unsupported configuration
#endif

// void lv_example_image_file(void)
// {
//     // 创建一个图像对象
//     lv_obj_t * img = lv_img_create(lv_scr_act());  // 在当前屏幕上创建图像对象

//     // 设置图像源为文件路径
//     lv_img_set_src(img, "A:/mnt/sd/image/4g_1.png"); // 替换为实际图片路径
//     if (lv_img_get_src(img) == NULL) {
//         printf("Failed to set image source\n");
//         return;
//     }
//     lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);     // 图像居中显示
// }

int main(void)
{
    lv_init();

    /*Linux display device init*/
    lv_linux_disp_init();

    /*Create a Demo*/
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();

    /* Create a GUI-Guider app */
    setup_ui(&g_ui);
    custom_init(&g_ui);

    // lv_example_image_file();

    /*Handle LVGL tasks*/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}
