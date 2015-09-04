# todo: this is duplicated from stm32_common. lots of this stuff is common
# to a large class of microcontrollers. figure out how to factor it nicely.
set(CMAKE_SYSTEM_NAME Generic)
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
set(FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")
#include_directories(${PROJECT_SOURCE_DIR}/systems/samv71_xplained_ultra-metal/libchip_samv7)
include_directories(${PROJECT_SOURCE_DIR}/systems/samv71_xplained_ultra-metal/samv7)
include_directories(${PROJECT_SOURCE_DIR}/systems/samv71_xplained_ultra-metal/cmsis)
# todo: include chip header here explicitly, so this can be shared by lots
# of SAM-based boards, like stm32_common does?
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-lc -lgcc -mthumb -static -Wl,--print-gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/map,--cref")
#set(CMAKE_EXE_LINKER_FLAGS "-lc -lgcc -mthumb -static -Wl,--no-gc-sections -Wl,--print-gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/map,--cref")
set(CMAKE_EXECUTABLE_SUFFIX .elf)

set(make_binfiles ON CACHE BOOL "build binaries from ELFs")
function(make_bin exe elf bin)
  #message("hello i will now turn ${elf} into ${bin}")
  add_custom_command(OUTPUT ${bin}
                     COMMAND arm-none-eabi-objcopy -O binary ${elf} ${bin}
                     DEPENDS ${elf}
                     COMMENT "creating ${bin}")
  add_custom_target(${exe}_bin ALL DEPENDS ${bin})

  add_custom_command(OUTPUT ${elf}.objdump
                     COMMAND arm-none-eabi-objdump -S -d ${elf} > ${elf}.objdump
                     DEPENDS ${elf}
                     COMMENT "disassembling ${elf}")
  add_custom_target(${exe}_objdump ALL DEPENDS ${elf}.objdump)
endfunction()
# add FPU stuff for fancy SAM7 chips
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -mthumb")
# todo: dig up linker script from an old project somewhere
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -T ${PROJECT_SOURCE_DIR}/systems/stm32_common/ld/stm32f4xx.ld")
