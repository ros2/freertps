cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

#set(freertps_cores  x86_64 arm_cm4f) # arm_cm7f)
#set(freertps_chips  x86_64 stm32f4 stm32f7)
#set(freertps_boards generic stm32f4_disco stm32f7_disco)
#set(freertps_oses   posix metal)

# combo meal is either (burger, fries, beverage) or (core, chip, board, os)
#set(freertps_combo x86_64-x86_64-generic-posix
#                   cm4-stm32f4-stm32f4_disco-metal 
#                   cm7-stm32f7-stm32f7_disco-metal)

#set(posix_INCLUDES posix)
#set(metal_INCLUDES metal)

include_directories(include)

function(freertps_add_executable exe srcs)
  add_executable(${exe} ${srcs})
  target_link_libraries(${exe} freertps ${SYSTEM_LIBS})
endfunction()

set(SYSTEM "SYSTEM" CACHE STRING "generic-posix")
message("system: ${SYSTEM}")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -g")
# these flags are common for any ARM Cortex-M chip with built-in FPU
set(ARMCM_FPU_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")

# set defines, compiler, and build support libraries specific to this system
add_subdirectory(systems/${SYSTEM})
#include("systems/${SYSTEM}.cmake")

# give the chip a chance to override the toolchain (compiler, flags, etc.)
#include("toolchains/${SYSTEM_CHIP}.cmake")

#set(x86_64_C_FLAGS "${COMMON_FLAGS}")
#set(arm_cm4f_C_FLAGS "${COMMON_FLAGS} ${ARMCM_FPU_FLAGS}")
#set(arm_cm7f_C_FLAGS "${COMMON_FLAGS} ${ARMCM_FPU_FLAGS}")

#set(x86_64_C_COMPILER gcc)
#set(arm_cm7f_C_COMPILER arm-none-eabi-gcc)

#include(${CMAKE_PROJECT_DIR}/systems/arm-

# make the portable library for this core
add_library(freertps STATIC discovery.c 
                            freertps.c 
                            id.c 
                            participant.c 
                            publisher.c 
                            sedp.c 
                            spdp.c 
                            subscription.c 
                            time.c 
                            udp.c)

message("apps: [${SYSTEM_APPS}]")
# build the applications for this system
foreach(app ${SYSTEM_APPS})
  message("configuring app [${app}]")
  add_subdirectory(apps/${app})
endforeach()

#include_directories(include)
#if (stm32)
#  #set(CMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)
#  include_directories(stm32)
#  include_directories(stm32/cmsis)
#  add_subdirectory(stm32)
#  add_subdirectory(metal)
#  set(plat_libs metal stm32)
#else()
#  include_directories(posix)
#  add_subdirectory(posix)
#  set(plat_libs freertps_posix)
#endif()

#target_link_libraries(freertps ${plat_libs})
#target_link_libraries(freertps metal stm32)

