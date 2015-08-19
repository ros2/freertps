cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

find_package(ament_cmake REQUIRED)

include_directories(include)
# in the future, consider target_include_directories() instead...
ament_export_include_directories(include)

function(freertps_add_executable exe srcs)
  add_executable(${exe} ${srcs})
  target_link_libraries(${exe} freertps -Wl,--start-group ${SYSTEM} ${SYSTEM_EXTRA_LIBS} ${SYSTEM_BONUS_LIBS} -Wl,--end-group)
  if(${make_binfiles})
    make_bin(${exe} ${CMAKE_CURRENT_BINARY_DIR}/${exe}.elf ${CMAKE_CURRENT_BINARY_DIR}/${exe}.bin)
  endif()
endfunction()

set(SYSTEM "native-posix" CACHE STRING "the target system name") 
message("system: [${SYSTEM}]")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -O2 -fPIC")
# ament_export_definitions()....in the future

# set up the toolchain for this system, include additional stuff, set
# more defines for clock speeds, etc etc
include(systems/${SYSTEM}/toolchain.cmake)

# build the non-portable support library for this system
add_subdirectory(systems/${SYSTEM})

# and any other required system libraries
foreach(syslib ${SYSTEM_EXTRA_LIBS})
  include(systems/${syslib})
endforeach()
message("SYSTEM_APPS = ${SYSTEM_APPS}")

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
target_link_libraries(freertps freertps_system_${SYSTEM})
ament_export_libraries(freertps)
ament_export_libraries(freertps_system_${SYSTEM})

# build the applications for this system
#string(REGEX REPLACE " " ";" SYSTEM_APPS ${SYSTEM_APPS})
#foreach(app ${SYSTEM_APPS})
#  add_subdirectory(apps/${app})
#endforeach()


ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  TARGETS freertps freertps_system_${SYSTEM}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

