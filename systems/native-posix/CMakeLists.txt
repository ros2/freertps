#add_library(native-posix STATIC time.c 
add_library(native-posix        time.c 
                                udp.c 
                                bswap.c 
                                system.c 
                                fake_imu.c 
                                fake_led.c)
set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "")
set(SYSTEM_BONUS_LIBS "m" CACHE STRING "") # not the best variable name
set(SYSTEM_APPS  "listener talker imu led" CACHE STRING "")
