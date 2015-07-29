set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "")
set(SYSTEM_APPS  "listener talker imu" CACHE STRING "")
set(SYSTEM_BONUS_LIBS "m" CACHE STRING "") # not the best variable name

add_library(native-posix STATIC time.c udp.c bswap.c system.c fake_imu.c)
