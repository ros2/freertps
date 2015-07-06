include(CMakeForceCompiler)
cmake_minimum_required(VERSION 3.0)
project(freertps)
include(${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)
#set(CMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)

include_directories(stm32)
include_directories(stm32/cmsis)
add_subdirectory(stm32)
add_subdirectory(metal)

include_directories(include)
add_library(freertps STATIC discovery.c freertps.c id.c participant.c publisher.c sedp.c spdp.c subscription.c time.c udp.c)
target_link_libraries(freertps metal stm32)

add_subdirectory(examples)
