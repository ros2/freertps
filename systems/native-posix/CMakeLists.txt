set(SYSTEM_CORE  "native")
set(SYSTEM_CHIP  "native")
set(SYSTEM_LIBS  "posix" CACHE STRING "system libraries")
set(SYSTEM_APPS  "listener" CACHE STRING "applications for this system")

add_library(posix STATIC time_posix.c udp_posix.c bswap_posix.c system.c)
