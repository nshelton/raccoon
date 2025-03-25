/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h" // Include the LCD header
#include "esp_timer.h"


static const char *TAG = "example";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing LCD");
    
    // Initialize LCD
    lcd_handle_t lcd = lcd_init();
    if (!lcd) {
        ESP_LOGE(TAG, "Failed to initialize LCD");
        return;
    }
    
    ESP_LOGI(TAG, "Display initialized successfully");

    int64_t last_us = 0;
    int64_t last_display_us = 0;
    
    while (1) {
        // Clear buffer before each redraw
        lcd_clear_buffer(lcd);
        
        // Draw text with mixed case
        
        // Get current time and display it
        char ms[32];
        uint32_t time_ms = esp_timer_get_time();

        snprintf(ms, sizeof(ms), "%lu", time_ms);
        lcd_draw_string(lcd, ms, 1, 0);
        
        int64_t diff = time_ms - last_us;
        snprintf(ms, sizeof(ms), "%lld dt", diff);
        lcd_draw_string(lcd, ms, 1, 10);
        last_us = time_ms;

        snprintf(ms, sizeof(ms), "%lldus display", last_display_us);
        lcd_draw_string(lcd, ms, 1, 20);

        uint32_t start = esp_timer_get_time();
        lcd_update(lcd);
        uint32_t end = esp_timer_get_time();
        last_display_us = end - start;
        

        // IMPORTANT: Yield control to other tasks to prevent watchdog timer errors
        vTaskDelay(1);
    }
    
    // In a real application, we would call lcd_deinit(lcd) when done using the LCD
    // lcd_deinit(lcd);
}
   
