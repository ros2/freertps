include_directories(../stm32_common)
include_directories(.)
add_library(stm32f4_disco-metal STATIC console.c 
                                       led.c 
                                       systime.c
                                       flash.c
                                       imu.c
                                       enet_init_pins.c
                                       usb_fs_init_pins.c
                                       ../stm32_common/stm32f4_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener talker talker_stm32_timer imu led" CACHE STRING "applications for this system")
