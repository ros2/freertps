include_directories(../stm32_common)
include_directories(.)
add_library(freertps_system_stm3240g_eval-metal console.c 
                                led.c 
                                systime.c
                                flash.c
                                imu.c
                                enet_init_pins.c
                                usb_fs_init_pins.c
                                ../stm32_common/stm32f4_vectors.c)
set(SYSTEM_EXTRA_LIBS  "stm32_common metal_common" CACHE STRING "extra system libs")
set(SYSTEM_APPS  "listener talker talker_stm32_timer led" CACHE STRING "applications for this system")
if (NOT freertps_standalone)
  ament_export_libraries(freertps_system_${SYSTEM})
endif()
install(
  TARGETS freertps_system_${SYSTEM}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
