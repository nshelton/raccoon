/*
 * LCD interface for SH1107 OLED display
 */

#pragma once

#include <stdint.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

// LCD configuration// I successfully ran this at 1MHz, but the sh1107 says it's rated to only 400kHz
//https://www.displayfuture.com/Display/datasheet/controller/SH1107.pdf
#define LCD_PIXEL_CLOCK_HZ  (400 * 1000)
#define LCD_I2C_HW_ADDR     0x3C
#define LCD_PIN_SDA         15
#define LCD_PIN_SCL         27
#define LCD_PIN_RST         -1

// Display dimensions
#define LCD_WIDTH           64
#define LCD_HEIGHT          128

// Font definitions
#define FONT_WIDTH          5
#define FONT_HEIGHT         7
#define FONT_BYTES_PER_CHAR 5

// Opaque handle type for LCD object
typedef struct lcd_dev_t* lcd_handle_t;

/**
 * @brief Create and initialize a new LCD instance
 * 
 * @return lcd_handle_t Handle to the LCD object (NULL if failed)
 */
lcd_handle_t lcd_init(void);

/**
 * @brief Free the LCD resources
 * 
 * @param lcd Handle to the LCD object
 */
void lcd_deinit(lcd_handle_t lcd);

/**
 * @brief Update the LCD with buffer contents
 * 
 * @param lcd Handle to the LCD object
 */
void lcd_update(lcd_handle_t lcd);

/**
 * @brief Draw a test pattern on the LCD
 * 
 * @param lcd Handle to the LCD object
 */
void lcd_draw_test_pattern(lcd_handle_t lcd);

/**
 * @brief Get the LCD buffer
 * 
 * @param lcd Handle to the LCD object
 * @return uint8_t* Pointer to the LCD buffer
 */
uint8_t* lcd_get_buffer(lcd_handle_t lcd);

/**
 * @brief Clear the LCD buffer
 * 
 * @param lcd Handle to the LCD object
 */
void lcd_clear_buffer(lcd_handle_t lcd);

/**
 * @brief Draw a single pixel on the LCD buffer
 * 
 * @param lcd Handle to the LCD object
 * @param x X coordinate
 * @param y Y coordinate
 */
void lcd_put_pixel(lcd_handle_t lcd, int x, int y);

/**
 * @brief Draw a character on the LCD buffer
 * 
 * @param lcd Handle to the LCD object
 * @param c Character to draw
 * @param x X coordinate
 * @param y Y coordinate
 */
void lcd_draw_char(lcd_handle_t lcd, char c, int x, int y);

/**
 * @brief Draw a string on the LCD buffer
 * 
 * @param lcd Handle to the LCD object
 * @param str String to draw
 * @param x X coordinate
 * @param y Y coordinate
 */
void lcd_draw_string(lcd_handle_t lcd, const char *str, int x, int y);
