include_directories(../stm32_common)
include_directories(.)
add_library(freertps_system_stm32f4_disco-metal console.c 
                                led.c 
                                systime.c
                                flash.c
                                imu.c
                                enet_init_pins.c
                                usb_fs_init_pins.c
                                ../stm32_common/stm32f4_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common metal_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener talker talker_stm32_timer imu led blink hello_world cpp_test" CACHE STRING "applications for this system")
set(SYSTEM_NO_ROSIDL_APPS "talker_no_rosidl listener_no_rosidl" CACHE STRING "")
if (NOT freertps_standalone)
  ament_export_libraries(freertps_system_${SYSTEM})
endif()
install(
  TARGETS freertps_system_${SYSTEM}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
