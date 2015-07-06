include(CMakeForceCompiler)
cmake_minimum_required(VERSION 2.8.12)
project(freertps)

# figure out a more principled way of adding more embedded targets
# once at least stm32 is working
option(stm32 "build for stm32?" OFF)
set(extra_libs "")

include_directories(include)
if (stm32)
  #set(CMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)
  include(${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)
  include_directories(stm32)
  include_directories(stm32/cmsis)
  add_subdirectory(stm32)
  add_subdirectory(metal)
  set(plat_libs metal stm32)
else()
  set(CMAKE_C_FLAGS "-std=gnu99 -Wall")
  include_directories(posix)
  add_subdirectory(posix)
  set(plat_libs freertps_posix)
endif()

add_library(freertps STATIC discovery.c freertps.c id.c participant.c publisher.c sedp.c spdp.c subscription.c time.c udp.c)
target_link_libraries(freertps ${plat_libs})
#target_link_libraries(freertps metal stm32)

add_subdirectory(examples)
