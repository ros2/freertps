set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "system extra libraries")
set(SYSTEM_APPS  "listener" CACHE STRING "applications for this system")

add_library(native-posix STATIC time_posix.c udp_posix.c bswap_posix.c system.c)
