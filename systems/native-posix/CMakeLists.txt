add_library(freertps_system_native-posix  time.c
                                          udp.c 
                                          bswap.c 
                                          system.c 
                                          fake_imu.c 
                                          fake_led.c
                                          ser.c)
target_link_libraries(freertps_system_${SYSTEM} freertps)
set(SYSTEM_EXTRA_LIBS  "" CACHE STRING "")
set(SYSTEM_BONUS_LIBS "m" CACHE STRING "") # not the best variable name
set(SYSTEM_APPS  "led talker listener imu" CACHE STRING "")
set(SYSTEM_NO_ROSIDL_APPS "talker_no_rosidl listener_no_rosidl standalone_listen_for_n standalone_talk_n" CACHE STRING "")
install(
  TARGETS freertps_system_${SYSTEM}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
