include_directories(../stm32_common)
add_library(stm32f7_disco-metal STATIC console_init.c)
#add_library(stm32f7_disco-metal STATIC ../stm32_common/pin.c
#                                       ../stm32_common/net.c
#                                       ../stm32_common/console.c
#                                       ../stm32_common/enet.c
#                                       ../stm32_common/udp_stm32.c
#                                       ../stm32_common/time_stm32.c
#                                       ../stm32_common/delay.c
#                                       ../stm32_common/led.c
#                                       ../stm32_common/startup.c
#                                       ../stm32_common/systime.c
#                                       ../stm32_common/stubs.c
#                                       ../stm32_common/system.c
#                                       ../stm32_common/bswap_stm32.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener" CACHE STRING "applications for this system")
