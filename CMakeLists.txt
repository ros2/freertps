cmake_minimum_required(VERSION 2.8.12)
#SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
#SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

project(freertps)

set(freertps_standalone OFF CACHE BOOL "standalone build")

# in the future, consider target_include_directories() instead...
if(NOT freertps_standalone)
  find_package(ament_cmake REQUIRED)
  ament_export_include_directories(include)
endif()
include_directories(include)

function(freertps_add_executable exe)
  #message("freertps_add_executable(${exe})")
  add_executable(${exe} ${ARGN})
  set_target_properties(${exe} PROPERTIES LINKER_LANGUAGE C)
  foreach(syslib ${SYSTEM_EXTRA_LIBS})
    #message("adding dependency on ${syslib}")
    add_dependencies(${exe} ${syslib})
    #include(systems/${syslib})
  endforeach()

  target_link_libraries(${exe} ${ROSIDL_LIBS} -Wl,--start-group freertps freertps_system_${SYSTEM} ${SYSTEM_EXTRA_LIBS} ${SYSTEM_BONUS_LIBS} -Wl,--end-group)
  if(${make_binfiles})
    make_bin(${exe} ${CMAKE_CURRENT_BINARY_DIR}/${exe}.elf ${CMAKE_CURRENT_BINARY_DIR}/${exe}.bin)
  endif()
endfunction()

set(SYSTEM "native-posix" CACHE STRING "the target system name")
message("system: [${SYSTEM}]")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -g -O2 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -fPIC")
# ament_export_definitions()....in the future

# set up the toolchain for this system, include additional stuff, set
# more defines for clock speeds, etc etc
include(systems/${SYSTEM}/toolchain.cmake)

# build the non-portable support library for this system
add_subdirectory(systems/${SYSTEM})

# and any other required system libraries
if(NOT "${SYSTEM_EXTRA_LIBS}" STREQUAL "")
  message("SYSTEM_APPS = ${SYSTEM_APPS}")
  string(REGEX REPLACE " " ";" SYSTEM_EXTRA_LIBS ${SYSTEM_EXTRA_LIBS})
  foreach(syslib ${SYSTEM_EXTRA_LIBS})
    #include(systems/${syslib})
    add_subdirectory(systems/${syslib})
  endforeach()
endif()

# build the portable library
add_library(freertps disco.c
                     freertps.c 
                     id.c 
                     part.c 
                     spdp.c 
                     sedp.c 
                     sub.c 
                     pub.c 
                     time.c 
                     udp.c
                     psm/ser.c)
target_link_libraries(freertps freertps_system_${SYSTEM})

include_directories(build/msgs)

if (NOT freertps_standalone)
  ament_export_libraries(freertps freertps_system_${SYSTEM})

  if (AMENT_ENABLE_TESTING)
    find_package(ament_cmake_gtest REQUIRED)

    ament_add_gtest(talker_listener_gtest "tests/talker_listener_test.cpp" TIMEOUT 30)
    if (TARGET talker_listener_gtest)
      ament_target_dependencies(talker_listener_gtest "freertps")
      target_include_directories(talker_listener_gtest PUBLIC include)
      target_link_libraries(talker_listener_gtest ${_AMENT_EXPORT_LIBRARY_TARGETS} ${SYSTEM_EXTRA_LIBS} ${SYSTEM_BONUS_LIBS})
    endif()
  endif()

  ament_package()
  install(
    DIRECTORY include/
    DESTINATION include
  )
  install(
    TARGETS freertps
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
else()
  file(GLOB msgs_SOURCES "build/msgs/src/*.c")
  if(msgs_SOURCES STREQUAL "" AND DEFINED ENV{AMENT_PREFIX_PATH})
    execute_process(COMMAND r2/mega_genmsg.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    file(GLOB msgs_SOURCES "build/msgs/src/*.c")
  endif()

  if (NOT msgs_SOURCES STREQUAL "")
    # try to build the apps that use ROS messages
    add_library(msgs ${msgs_SOURCES})
    set(ROSIDL_LIBS "msgs" CACHE STRING "")
    # build the applications for this system
    string(REGEX REPLACE " " ";" SYSTEM_APPS ${SYSTEM_APPS})
    foreach(app ${SYSTEM_APPS})
      add_subdirectory(apps/${app})
    endforeach()
  else()
    set(ROSIDL_LIBS "" CACHE STRING "") # no ROS IDL library exists; don't try
  endif()

  # now, build the standalone apps. this lets at least some things build
  # on systems without the ROS2 messages installed
  if(DEFINED SYSTEM_NO_ROSIDL_APPS)
    string(REGEX REPLACE " " ";" SYSTEM_NO_ROSIDL_APPS ${SYSTEM_NO_ROSIDL_APPS})
    foreach(app ${SYSTEM_NO_ROSIDL_APPS})
      add_subdirectory(apps/${app})
    endforeach()
  endif()
endif()
