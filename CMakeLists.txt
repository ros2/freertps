cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

include_directories(include)

function(freertps_add_executable exe srcs)
  add_executable(${exe} ${srcs})
  target_link_libraries(${exe} freertps ${SYSTEM_LIBS})
  if(${make_binfiles})
    make_bin(${exe} ${CMAKE_CURRENT_BINARY_DIR}/${exe}.elf ${CMAKE_CURRENT_BINARY_DIR}/${exe}.bin)
  endif()
endfunction()

set(SYSTEM "SYSTEM" CACHE STRING "generic-posix")
message("system: ${SYSTEM}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -g")

# set up the toolchain for this system
include(systems/${SYSTEM}/toolchain.cmake)

# build the non-portable support library for this system
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

# build the applications for this system
foreach(app ${SYSTEM_APPS})
  add_subdirectory(apps/${app})
endforeach()
