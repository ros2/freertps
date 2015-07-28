include_directories(../stm32_common)
add_library(stm32f4_disco-metal STATIC console.c 
                                       led.c 
                                       systime.c
                                       flash.c
                                       enet_init_pins.c
                                       ../stm32_common/stm32f4_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener talker talker_stm32_timer" CACHE STRING "applications for this system")
