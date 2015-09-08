include_directories(../stm32_common)
add_library(freertps_system_stm32f7_disco-metal console.c 
                                       led.c 
                                       systime.c
                                       flash.c
                                       enet_init_pins.c
                                       dcmi.c
                                       i2c.c
                                       camera.c
                                       camera_init_pins.c
                                       lcd.c
                                       lcd_init_pins.c
                                       usb_fs_init_pins.c
                                       ../stm32_common/stm32f7_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common metal_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener talker talker_stm32_timer cam_dm_130" CACHE STRING "applications for this system")
#set(SYSTEM_APPS  "listener talker talker_stm32_timer cam_dm_130" CACHE STRING "applications for this system")
