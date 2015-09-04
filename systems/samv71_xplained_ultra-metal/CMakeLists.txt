add_library(freertps_system_samv71_xplained_ultra-metal
            startup.c
            led.c)
#            console.c
#            systime.c
#            flash.c
#            enet_init_pins.c
#            dcmi.c
#            i2c.c
#            camera.c
#            camera_init_pins.c
#            lcd.c
#            lcd_init_pins.c
#            usb_fs_init_pins.c
#            vectors.c)
#set(SYSTEM_EXTRA_LIBS  "stm32_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "blink" CACHE STRING "applications for this system")
#set(SYSTEM_APPS  "listener talker talker_stm32_timer cam_dm_130" CACHE STRING "applications for this system")
#set(SYSTEM_APPS  "listener talker talker_stm32_timer cam_dm_130" CACHE STRING "applications for this system")
