// /*
//  * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: CC0-1.0
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "lcd.h" // Include the LCD header
// #include "esp_timer.h"


// static const char *TAG = "example";

// void app_main(void)
// {
//     ESP_LOGI(TAG, "Initializing LCD");
    
//     // Initialize LCD
//     lcd_handle_t lcd = lcd_init();
//     if (!lcd) {
//         ESP_LOGE(TAG, "Failed to initialize LCD");
//         return;
//     }
    
//     ESP_LOGI(TAG, "Display initialized successfully");

//     int64_t last_us = 0;
//     int64_t last_display_us = 0;
    
//     while (1) {
//         // Clear buffer before each redraw
//         lcd_clear_buffer(lcd);
        
//         // Draw text with mixed case
        
//         // Get current time and display it
//         char ms[32];
//         uint32_t time_ms = esp_timer_get_time();

//         snprintf(ms, sizeof(ms), "%lu", time_ms);
//         lcd_draw_string(lcd, ms, 1, 0);
        
//         int64_t diff = time_ms - last_us;
//         snprintf(ms, sizeof(ms), "%lld dt", diff);
//         lcd_draw_string(lcd, ms, 1, 10);
//         last_us = time_ms;

//         snprintf(ms, sizeof(ms), "%lldus display", last_display_us);
//         lcd_draw_string(lcd, ms, 1, 20);

//         uint32_t start = esp_timer_get_time();
//         lcd_update(lcd);
//         uint32_t end = esp_timer_get_time();
//         last_display_us = end - start;
        

//         // IMPORTANT: Yield control to other tasks to prevent watchdog timer errors
//         vTaskDelay(1);
//     }
    
//     // In a real application, we would call lcd_deinit(lcd) when done using the LCD
//     // lcd_deinit(lcd);
// }
   

/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "bt/bt_app_core.h"
#include "bt/bt_app_av.h"
#include "bt/bt_main.c"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"


/*******************************
 * MAIN ENTRY POINT
 ******************************/

void app_main(void)
{
    char bda_str[18] = {0};
    /* initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /*
     * This example only uses the functions of Classical Bluetooth.
     * So release the controller memory for Bluetooth Low Energy.
     */
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
#if (CONFIG_EXAMPLE_A2DP_SINK_SSP_ENABLED == false)
    bluedroid_cfg.ssp_en = false;
#endif
    if ((err = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }

#if (CONFIG_EXAMPLE_A2DP_SINK_SSP_ENABLED == true)
    /* set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /* set default parameters for Legacy Pairing (use fixed pin code 1234) */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);

    ESP_LOGI(BT_AV_TAG, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
    bt_app_task_start_up();
    /* bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
}
