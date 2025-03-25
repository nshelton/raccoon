/*
 * Font and bitmap drawing functions for OLED displays
 */

#include "lcd.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lcd_sh1107.h"
#include "esp_heap_caps.h"
#include <string.h>

static const char *TAG = "lcd";

// I2C settings
#define I2C_BUS_PORT 0
#define I2C_CMD_BITS 8
#define I2C_PARAM_BITS 8

// Define the LCD resolution here since it's needed for pixel calculations
#define ROTATED_LCD_H_RES 64
#define ROTATED_LCD_V_RES 128

// LCD device structure
struct lcd_dev_t {
    esp_lcd_panel_handle_t panel_handle;
    i2c_master_bus_handle_t i2c_bus;
    uint8_t *buffer;
    int buffer_size;
};

// Character definition: each byte is a column, bits 0-6 are pixels from top to bottom
const uint8_t font5x7[][FONT_BYTES_PER_CHAR] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    // " (34)
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // # (35)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // $ (36)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    // % (37)
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // & (38)
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // ' (39)
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // ( (40)
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    // ) (41)
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    // * (42)
    {0x14, 0x08, 0x3E, 0x08, 0x14},
    // + (43)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // , (44)
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // - (45)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // . (46)
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // / (47)
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1 (49)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : (58)
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // ; (59)
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // < (60)
    {0x08, 0x14, 0x22, 0x41, 0x00},
    // = (61)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // > (62)
    {0x00, 0x41, 0x22, 0x14, 0x08},
    // ? (63)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // @ (64)
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A (65)
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    // B (66)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F (70)
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // G (71)
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I (73)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M (77)
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W (87)
    {0x3F, 0x40, 0x38, 0x40, 0x3F},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y (89)
    {0x07, 0x08, 0x70, 0x08, 0x07},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [ (91)
    {0x00, 0x7F, 0x41, 0x41, 0x00},
    // \ (92)
    {0x02, 0x04, 0x08, 0x10, 0x20},
    // ] (93)
    {0x00, 0x41, 0x41, 0x7F, 0x00},
    // ^ (94)
    {0x04, 0x02, 0x01, 0x02, 0x04},
    // _ (95)
    {0x40, 0x40, 0x40, 0x40, 0x40},
    // ` (96)
    {0x00, 0x01, 0x02, 0x04, 0x00},
    // a (97)
    {0x20, 0x54, 0x54, 0x54, 0x78},
    // b (98)
    {0x7F, 0x48, 0x44, 0x44, 0x38},
    // c (99)
    {0x38, 0x44, 0x44, 0x44, 0x20},
    // d (100)
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    // e (101)
    {0x38, 0x54, 0x54, 0x54, 0x18},
    // f (102)
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    // g (103)
    {0x0C, 0x52, 0x52, 0x52, 0x3E},
    // h (104)
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    // i (105)
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    // j (106)
    {0x20, 0x40, 0x44, 0x3D, 0x00},
    // k (107)
    {0x7F, 0x10, 0x28, 0x44, 0x00},
    // l (108)
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    // m (109)
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    // n (110)
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    // o (111)
    {0x38, 0x44, 0x44, 0x44, 0x38},
    // p (112)
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    // q (113)
    {0x08, 0x14, 0x14, 0x18, 0x7C},
    // r (114)
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    // s (115)
    {0x48, 0x54, 0x54, 0x54, 0x20},
    // t (116)
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    // u (117)
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    // v (118)
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    // w (119)
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    // x (120)
    {0x44, 0x28, 0x10, 0x28, 0x44},
    // y (121)
    {0x0C, 0x50, 0x50, 0x50, 0x3C},
    // z (122)
    {0x44, 0x64, 0x54, 0x4C, 0x44},
    // { (123)
    {0x00, 0x08, 0x36, 0x41, 0x00},
    // | (124)
    {0x00, 0x00, 0x7F, 0x00, 0x00},
    // } (125)
    {0x00, 0x41, 0x36, 0x08, 0x00},
    // ~ (126)
    {0x10, 0x08, 0x08, 0x10, 0x08}
};

void lcd_put_pixel_rotated(lcd_handle_t lcd, int x, int y) {
    if (x >= ROTATED_LCD_H_RES || y >= ROTATED_LCD_V_RES || !lcd || !lcd->buffer || x < 0 || y < 0) {
        return;
    }

    // y = ROTATED_LCD_V_RES - y;
    lcd->buffer[(y / 8) * ROTATED_LCD_H_RES + x] |= (1 << (y % 8));
}

void lcd_put_pixel(lcd_handle_t lcd, int x, int y) {
    lcd_put_pixel_rotated(lcd, ROTATED_LCD_H_RES - y - 1, x);
}

void lcd_draw_char(lcd_handle_t lcd, char c, int x, int y) {
    if (!lcd || !lcd->buffer) {
        return;
    }
    
    // Only handle printable ASCII characters
    if (c < ' ' || c > '~') {
        c = '?'; // Use question mark for undefined characters
    }

    // Calculate the index in our font array
    int charIndex = c - ' ';
    
    // Draw each column of the character
    for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t columnData = font5x7[charIndex][col];
        
        // Draw each pixel in the column
        for (int row = 0; row < FONT_HEIGHT; row++) {
            if (columnData & (1 << row)) {
                lcd_put_pixel(lcd, x + col, y + row);
            }
        }
    }
}

void lcd_draw_string(lcd_handle_t lcd, const char *str, int x, int y) {
    if (!lcd || !lcd->buffer || !str) {
        return;
    }
    
    int currentX = x;
    
    while (*str) {
        lcd_draw_char(lcd, *str++, currentX, y);
        currentX += FONT_WIDTH + 1; // Add 1 pixel spacing between characters
    }
}

void lcd_clear_buffer(lcd_handle_t lcd) {
    if (lcd && lcd->buffer && lcd->buffer_size > 0) {
        memset(lcd->buffer, 0, lcd->buffer_size);
    }
}

lcd_handle_t lcd_init(void)
{
    ESP_LOGI(TAG, "Initializing LCD");
    
    // Allocate memory for the LCD device structure
    lcd_handle_t lcd = (lcd_handle_t)heap_caps_calloc(1, sizeof(struct lcd_dev_t), MALLOC_CAP_DEFAULT);
    if (lcd == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for LCD device structure");
        return NULL;
    }
    
    // Initialize I2C bus
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = LCD_PIN_SDA,
        .scl_io_num = LCD_PIN_SCL,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t ret = i2c_new_master_bus(&bus_config, &lcd->i2c_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }

    // Initialize panel IO
    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = LCD_I2C_HW_ADDR,
        .scl_speed_hz = LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,            // According to SSD1306 datasheet
        .lcd_cmd_bits = I2C_CMD_BITS,        // According to SSD1306 datasheet
        .lcd_param_bits = I2C_PARAM_BITS,    // According to SSD1306 datasheet
        .dc_bit_offset = 0,                  // According to SH1107 datasheet
        .flags = {
            .disable_control_phase = 1,
        }
    };
    ret = esp_lcd_new_panel_io_i2c(lcd->i2c_bus, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }

    // Initialize SH1107 panel driver
    ESP_LOGI(TAG, "Install SH1107 panel driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = LCD_PIN_RST,
    };
    ret = esp_lcd_new_panel_sh1107(io_handle, &panel_config, &lcd->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }

    // Initialize the panel
    ret = esp_lcd_panel_reset(lcd->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset panel: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }
    
    ret = esp_lcd_panel_init(lcd->panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize panel: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }
    
    ret = esp_lcd_panel_disp_on_off(lcd->panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on display: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }
    
    ret = esp_lcd_panel_invert_color(lcd->panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to invert color: %s", esp_err_to_name(ret));
        lcd_deinit(lcd);
        return NULL;
    }
    
    // Allocate the display buffer
    lcd->buffer_size = LCD_WIDTH * LCD_HEIGHT / 8;
    lcd->buffer = heap_caps_malloc(lcd->buffer_size, MALLOC_CAP_DEFAULT);
    if (lcd->buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for display buffer");
        lcd_deinit(lcd);
        return NULL;
    }
    
    // Clear the buffer initially
    lcd_clear_buffer(lcd);
    
    ESP_LOGI(TAG, "LCD initialized successfully");
    return lcd;
}

void lcd_deinit(lcd_handle_t lcd)
{
    if (lcd) {
        // Free buffer
        if (lcd->buffer) {
            heap_caps_free(lcd->buffer);
        }
        
        // Delete panel handle
        if (lcd->panel_handle) {
            esp_lcd_panel_del(lcd->panel_handle);
        }
        
        // Delete I2C bus
        if (lcd->i2c_bus) {
            i2c_del_master_bus(lcd->i2c_bus);
        }
        
        // Free the LCD handle itself
        heap_caps_free(lcd);
    }
}

void lcd_update(lcd_handle_t lcd)
{
    if (!lcd || !lcd->panel_handle || !lcd->buffer) {
        ESP_LOGE(TAG, "Invalid LCD handle or resources");
        return;
    }
    
    // Send buffer to display
    esp_lcd_panel_draw_bitmap(lcd->panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, lcd->buffer);
}
 
uint8_t* lcd_get_buffer(lcd_handle_t lcd)
{
    if (!lcd) {
        return NULL;
    }
    return lcd->buffer;
}