#include "gd32g5x3.h"
#include <stdlib.h>
#include "lcd_driver.h"
#include "systick.h"

#ifdef H_VIEW
#define X_MAX_PIXEL         (uint16_t)320
#define Y_MAX_PIXEL         (uint16_t)240
#else
#define X_MAX_PIXEL         (uint16_t)240
#define Y_MAX_PIXEL         (uint16_t)320
#endif

#define LCD_CS_PORT         GPIOB
#define LCD_CS_PIN          GPIO_PIN_12
#define LCD_DC_PORT         GPIOC
#define LCD_DC_PIN          GPIO_PIN_8
#define LCD_RST_PORT        GPIOA
#define LCD_RST_PIN         GPIO_PIN_5

#define LCD_CS_SET          gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN)
#define LCD_CS_CLR          gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN)
#define LCD_RS_SET          gpio_bit_set(LCD_DC_PORT, LCD_DC_PIN)
#define LCD_RS_CLR          gpio_bit_reset(LCD_DC_PORT, LCD_DC_PIN)
#define LCD_RST_SET         gpio_bit_set(LCD_RST_PORT, LCD_RST_PIN)
#define LCD_RST_CLR         gpio_bit_reset(LCD_RST_PORT, LCD_RST_PIN)

static uint8_t spi_write_byte(uint32_t spi_periph, uint8_t byte);
static void spi1_init(void);
static void lcd_write_index(uint8_t index);
static void lcd_write_data(uint8_t data);
static void lcd_write_data_16bit(uint8_t datah, uint8_t datal);
static void lcd_reset(void);

static uint8_t spi_write_byte(uint32_t spi_periph, uint8_t byte)
{
    while(RESET == (SPI_STAT(spi_periph) & SPI_FLAG_TBE)) {
    }
    SPI_DATA(spi_periph) = byte;

    while(RESET == (SPI_STAT(spi_periph) & SPI_FLAG_RBNE)) {
    }
    return SPI_DATA(spi_periph);
}

static void spi1_init(void)
{
    spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI1);

    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, GPIO_PIN_12);
    LCD_CS_SET;

    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, GPIO_PIN_8);
    LCD_RS_SET;

    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_85MHZ, GPIO_PIN_5);
    LCD_RST_SET;

    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_4;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);

    spi_fifo_access_size_config(SPI1, SPI_BYTE_ACCESS);
    spi_crc_polynomial_set(SPI1, 7);
    spi_enable(SPI1);
}

static void lcd_write_index(uint8_t index)
{
    LCD_RS_CLR;
    LCD_CS_CLR;
    spi_write_byte(SPI1, index);
}

static void lcd_write_data(uint8_t data)
{
    LCD_RS_SET;
    spi_write_byte(SPI1, data);
}

static void lcd_write_data_16bit(uint8_t datah, uint8_t datal)
{
    lcd_write_data(datah);
    lcd_write_data(datal);
}

static void lcd_reset(void)
{
    LCD_RST_CLR;
    delay_1ms(100);
    LCD_RST_SET;
    delay_1ms(50);
}

void lcd_init(void)
{
    spi1_init();
    lcd_reset();

    lcd_write_index(0xCB);
    lcd_write_data(0x39);
    lcd_write_data(0x2C);
    lcd_write_data(0x00);
    lcd_write_data(0x34);
    lcd_write_data(0x02);

    lcd_write_index(0xCF);
    lcd_write_data(0x00);
    lcd_write_data(0xC1);
    lcd_write_data(0x30);

    lcd_write_index(0xE8);
    lcd_write_data(0x85);
    lcd_write_data(0x00);
    lcd_write_data(0x78);

    lcd_write_index(0xEA);
    lcd_write_data(0x00);
    lcd_write_data(0x00);

    lcd_write_index(0xED);
    lcd_write_data(0x64);
    lcd_write_data(0x03);
    lcd_write_data(0x12);
    lcd_write_data(0x81);

    lcd_write_index(0xF7);
    lcd_write_data(0x20);

    lcd_write_index(0xC0);
    lcd_write_data(0x23);

    lcd_write_index(0xC1);
    lcd_write_data(0x10);

    lcd_write_index(0xC5);
    lcd_write_data(0x3e);
    lcd_write_data(0x28);

    lcd_write_index(0xC7);
    lcd_write_data(0x86);

    lcd_write_index(0x36);
#ifdef H_VIEW
    lcd_write_data(0xE8);
#else
    lcd_write_data(0x48);
#endif

    lcd_write_index(0x3A);
    lcd_write_data(0x55);

    lcd_write_index(0xB1);
    lcd_write_data(0x00);
    lcd_write_data(0x18);

    lcd_write_index(0xB6);
    lcd_write_data(0x08);
    lcd_write_data(0x82);
    lcd_write_data(0x27);

    lcd_write_index(0xF2);
    lcd_write_data(0x00);

    lcd_write_index(0x26);
    lcd_write_data(0x01);

    lcd_write_index(0xE0);
    lcd_write_data(0x0F);
    lcd_write_data(0x31);
    lcd_write_data(0x2B);
    lcd_write_data(0x0C);
    lcd_write_data(0x0E);
    lcd_write_data(0x08);
    lcd_write_data(0x4E);
    lcd_write_data(0xF1);
    lcd_write_data(0x37);
    lcd_write_data(0x07);
    lcd_write_data(0x10);
    lcd_write_data(0x03);
    lcd_write_data(0x0E);
    lcd_write_data(0x09);
    lcd_write_data(0x00);

    lcd_write_index(0xE1);
    lcd_write_data(0x00);
    lcd_write_data(0x0E);
    lcd_write_data(0x14);
    lcd_write_data(0x03);
    lcd_write_data(0x11);
    lcd_write_data(0x07);
    lcd_write_data(0x31);
    lcd_write_data(0xC1);
    lcd_write_data(0x48);
    lcd_write_data(0x08);
    lcd_write_data(0x0F);
    lcd_write_data(0x0C);
    lcd_write_data(0x31);
    lcd_write_data(0x36);
    lcd_write_data(0x0F);

    lcd_write_index(0x11);
    delay_1ms(120);

    lcd_write_index(0x29);
    lcd_write_index(0x2C);

    LCD_CS_SET;
}

void lcd_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    LCD_CS_CLR;

    lcd_write_index(0x2A);
    lcd_write_data_16bit(x_start >> 8, x_start);
    lcd_write_data_16bit(x_end >> 8, x_end);

    lcd_write_index(0x2B);
    lcd_write_data_16bit(y_start >> 8, y_start);
    lcd_write_data_16bit(y_end >> 8, y_end);

    lcd_write_index(0x2C);
    LCD_CS_SET;
}

void lcd_set_xy(uint16_t x, uint16_t y)
{
    LCD_CS_CLR;

    lcd_write_index(0x2A);
    lcd_write_data_16bit(x >> 8, x);

    lcd_write_index(0x2B);
    lcd_write_data_16bit(y >> 8, y);

    lcd_write_index(0x2C);
}

void gui_draw_point(uint16_t x, uint16_t y, uint16_t data)
{
    lcd_set_xy(x, y);
    LCD_RS_SET;
    spi_write_byte(SPI1, data >> 8);
    spi_write_byte(SPI1, data);
    LCD_CS_SET;
}

void lcd_clear(uint16_t color)
{
    uint32_t i;
    uint32_t m;
    uint32_t pixel_count = (uint32_t)X_MAX_PIXEL * (uint32_t)Y_MAX_PIXEL;
    uint16_t *temp_buf = NULL;

    if(pixel_count > 0U) {
        temp_buf = (uint16_t *)malloc(pixel_count * sizeof(uint16_t));
        if(NULL != temp_buf) {
            for(i = 0; i < pixel_count; i++) {
                temp_buf[i] = color;
            }
            lcd_write_pixels(0, 0, X_MAX_PIXEL - 1, Y_MAX_PIXEL - 1, temp_buf);
            free(temp_buf);
            return;
        }
    }

    lcd_set_region(0, 0, X_MAX_PIXEL - 1, Y_MAX_PIXEL - 1);

    LCD_RS_SET;
    LCD_CS_CLR;

    for(i = 0; i < Y_MAX_PIXEL; i++) {
        for(m = 0; m < X_MAX_PIXEL; m++) {
            spi_write_byte(SPI1, color >> 8);
            spi_write_byte(SPI1, color);
        }
    }

    LCD_CS_SET;
}

void lcd_send_pixels(const uint16_t *buf, uint32_t count)
{
    uint32_t i;

    if(NULL == buf || 0U == count) {
        return;
    }

    LCD_RS_SET;
    LCD_CS_CLR;

    for(i = 0U; i < count; i++) {
        uint16_t color = buf[i];
        spi_write_byte(SPI1, (uint8_t)(color >> 8));
        spi_write_byte(SPI1, (uint8_t)(color & 0xFF));
    }

    LCD_CS_SET;
}

void lcd_write_pixels(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *buf)
{
    uint32_t x_count = (uint32_t)(x_end - x_start + 1U);
    uint32_t y_count = (uint32_t)(y_end - y_start + 1U);
    uint32_t pixel_count = x_count * y_count;
    uint32_t i;

    if(NULL == buf || 0U == pixel_count) {
        return;
    }

    lcd_set_region(x_start, y_start, x_end, y_end);
    LCD_RS_SET;
    LCD_CS_CLR;

    for(i = 0U; i < pixel_count; i++) {
        uint16_t color = buf[i];
        spi_write_byte(SPI1, (uint8_t)(color >> 8));
        spi_write_byte(SPI1, (uint8_t)(color & 0xFF));
    }

    LCD_CS_SET;
}
