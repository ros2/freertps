include_directories(../stm32_common)
add_library(freertps_system_stm32f7_disco-metal console.c 
                                       led.c 
                                       systime.c
                                       flash.c
                                       enet_init_pins.c
                                       i2c.c
                                       ../stm32_common/stm32f7_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common metal_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "led listener talker talker_stm32_timer" CACHE STRING "applications for this system")
set(SYSTEM_NO_ROSIDL_APPS "talker_no_rosidl listener_no_rosidl" CACHE STRING "")
