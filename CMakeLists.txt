cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

include_directories(include)
ament_export_include_directories(include)

function(freertps_add_executable exe srcs)
  add_executable(${exe} ${srcs})
  target_link_libraries(${exe} freertps -Wl,--start-group ${SYSTEM} ${SYSTEM_EXTRA_LIBS} ${SYSTEM_BONUS_LIBS} -Wl,--end-group)
  if(${make_binfiles})
    make_bin(${exe} ${CMAKE_CURRENT_BINARY_DIR}/${exe}.elf ${CMAKE_CURRENT_BINARY_DIR}/${exe}.bin)
  endif()
endfunction()

message("system: ${SYSTEM}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -O2")

# set up the toolchain for this system
include(systems/${SYSTEM}/toolchain.cmake)

# build the non-portable support library for this system
add_subdirectory(systems/${SYSTEM})

# and any other required system libraries
foreach(syslib ${SYSTEM_EXTRA_LIBS})
  add_subdirectory(systems/${syslib})
endforeach()

# build the portable library
add_library(freertps STATIC disco.c 
                            freertps.c 
                            id.c 
                            part.c 
                            spdp.c 
                            sedp.c 
                            sub.c 
                            pub.c 
                            time.c 
                            udp.c)

# build the applications for this system
#string(REGEX REPLACE " " ";" SYSTEM_APPS ${SYSTEM_APPS})
#foreach(app ${SYSTEM_APPS})
#  add_subdirectory(apps/${app})
#endforeach()

ament_package()

install(
  DIRECTORY include
  DESTINATION include
)
install(
  TARGETS freertps
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

