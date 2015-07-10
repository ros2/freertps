add_executable(listener listener.c)
target_link_libraries(listener ${plat_libs} freertps ${plat_libs})

if (stm32)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/listener.bin
                     COMMAND arm-none-eabi-objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/listener.elf ${CMAKE_CURRENT_BINARY_DIR}/listener.bin
                     DEPENDS listener
                     COMMENT "creating listener.bin")
  add_custom_target(listener_bin ALL 
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/listener.bin)
endif()
