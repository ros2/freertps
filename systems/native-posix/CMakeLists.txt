add_library(freertps_system_native-posix  time.c
                                          udp.c 
                                          bswap.c 
                                          system.c 
                                          fake_imu.c 
                                          fake_led.c)
target_link_libraries(freertps_system_${SYSTEM} freertps)
set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "")
set(SYSTEM_BONUS_LIBS "m" CACHE STRING "") # not the best variable name
set(SYSTEM_APPS  "listener talker imu led cpp_test" CACHE STRING "")
install(
  TARGETS freertps_system_${SYSTEM}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
