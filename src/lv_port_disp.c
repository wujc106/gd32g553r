#include "lvgl/lvgl.h"
#include "lcd_driver.h"
#include "systick.h"

#define LV_PORT_LCD_WIDTH      240
#define LV_PORT_LCD_HEIGHT     320
#define LV_PORT_BUF_SIZE       ((LV_PORT_LCD_WIDTH * LV_PORT_LCD_HEIGHT) / 10U)

static uint8_t lv_buf[LV_PORT_BUF_SIZE * sizeof(lv_color_t)];

static void lv_port_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    const uint16_t *buf16 = (const uint16_t *)px_map;
    uint16_t x_start = (uint16_t)area->x1;
    uint16_t y_start = (uint16_t)area->y1;
    uint16_t x_end = (uint16_t)area->x2;
    uint16_t y_end = (uint16_t)area->y2;
    uint32_t pixel_count = (uint32_t)(x_end - x_start + 1U) * (uint32_t)(y_end - y_start + 1U);

    if(pixel_count > 0U && buf16 != NULL) {
        lcd_write_pixels(x_start, y_start, x_end, y_end, buf16);
    }

    lv_display_flush_ready(disp);
}

void lv_port_init(void)
{
    lv_init();
    lv_tick_set_cb(millis_get);

    lv_display_t *disp = lv_display_create(LV_PORT_LCD_WIDTH, LV_PORT_LCD_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lv_port_flush_cb);
    lv_display_set_buffers(disp, lv_buf, NULL, sizeof(lv_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_label_set_text(label, "Hello World");
    lv_obj_center(label);

    lcd_init();
    lcd_clear(0x0000);
    lv_refr_now(disp);
}

void lv_port_task(void)
{
    lv_timer_handler();
}
