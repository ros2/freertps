set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "")
set(SYSTEM_APPS  "listener talker" CACHE STRING "")
set(SYSTEM_BONUS_LIBS "m" CACHE STRING "") # not the best variable name

add_library(native-posix STATIC time_posix.c udp_posix.c bswap_posix.c system.c)
