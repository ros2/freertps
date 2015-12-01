add_library(freertps_system_samv71_xplained_ultra-metal
            vectors.c
            startup.c
            console.c
            led.c
            enet_mac.c
            systime.c
            time.c
            pin.c
            cam.c
            i2c.c
            usb_mac.c)
            #udp.c
#            flash.c
#            dcmi.c
#            i2c.c
#            camera_init_pins.c
#            lcd.c
#            lcd_init_pins.c
set(SYSTEM_EXTRA_LIBS  "metal_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS "usb_test cam" CACHE STRING "applications")
set(SYSTEM_NO_ROSIDL_APPS "blink talker_no_rosidl listener_no_rosidl" CACHE STRING "")
#set(SYSTEM_APPS "usb_test talker listener enet_test cam" CACHE STRING "applications")
#set(SYSTEM_APPS "blink hello_world talker listener systime_test enet_test" CACHE STRING "applications")
#set(SYSTEM_APPS  "listener talker talker_stm32_timer cam_dm_130" CACHE STRING "applications for this system")
