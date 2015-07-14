#set(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_C_COMPILER arm-none-eabi-gcc)
#set(CMAKE_RANLIB CACHE STRING arm-none-eabi-ranlib)
#set(CMAKE_AR CACHE STRING arm-none-eabi-ar)
#include(CMakeForceCompiler)
#CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
set(FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")
include_directories(${PROJECT_SOURCE_DIR}/systems/stm32_common/cmsis)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-lc -lgcc -mthumb -static -Wl,--no-gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/map,--cref")
set(CMAKE_EXECUTABLE_SUFFIX .elf)

set(make_binfiles ON CACHE BOOL "build binaries from ELFs")
function(make_bin exe elf bin)
  #message("hello i will now turn ${elf} into ${bin}")
  add_custom_command(OUTPUT ${bin}
                     COMMAND arm-none-eabi-objcopy -O binary ${elf} ${bin}
                     DEPENDS ${elf}
                     COMMENT "creating ${bin}")
  add_custom_target(${exe}_bin ALL DEPENDS ${bin})
endfunction()
