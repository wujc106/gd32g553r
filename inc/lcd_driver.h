#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>

void lcd_init(void);
void lcd_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void lcd_set_xy(uint16_t x, uint16_t y);
void gui_draw_point(uint16_t x, uint16_t y, uint16_t data);
void lcd_clear(uint16_t color);
void lcd_send_pixels(const uint16_t *buf, uint32_t count);
void lcd_write_pixels(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *buf);

#endif /* LCD_DRIVER_H */
