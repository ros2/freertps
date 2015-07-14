cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

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

# set up toolchain for this system
include(systems/${SYSTEM}/toolchain.cmake)

# set defines, compiler, and build support libraries specific to this system
add_subdirectory(systems/${SYSTEM})

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
