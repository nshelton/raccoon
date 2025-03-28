idf_component_register(
    SRCS 
    "main.c" 
    "lcd.c"
    "bt/bt_app_av.c"
    "bt/bt_main.c"
    "bt/bt_app_core.c"
    INCLUDE_DIRS 
    "."
    REQUIRES "bt" # required to be able to #include esp_bt.h
    REQUIRES nvs_flash

)
