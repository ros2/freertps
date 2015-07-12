cmake_minimum_required(VERSION 2.8.12)
project(freertps C)

set(freertps_platforms x86_64-posix
                       stm32f4-metal 
                       stm32f7-metal)

function(platform_arch platform arch)
  string(REPLACE "-" ";" components ${platform})
  list(GET components 0 arch_ele)
  set(${arch} ${arch_ele} PARENT_SCOPE)
endfunction()

function(platform_os platform os)
  string(REPLACE "-" ";" components ${platform})
  list(GET components 1 os_ele)
  set(${os} ${os_ele} PARENT_SCOPE)
endfunction()

#set(posix_INCLUDES posix)
#set(metal_INCLUDES metal)

include_directories(include)

foreach(platform IN LISTS freertps_platforms)
  platform_arch("${platform}" arch)
  platform_os("${platform}" os)
  message("creating library target for ${platform} which has arch ${arch} and os ${os}")
  #add_library(freertps_${platform} STATIC discovery.c freertps.c id.c participant.c publisher.c sedp.c spdp.c subscription.c time.c udp.c)
endforeach()

#option(stm32 "build for stm32?" OFF)

#include_directories(include)
#if (stm32)
#  #set(CMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/stm32_toolchain.cmake)
#  include_directories(stm32)
#  include_directories(stm32/cmsis)
#  add_subdirectory(stm32)
#  add_subdirectory(metal)
#  set(plat_libs metal stm32)
#else()
#  set(CMAKE_C_FLAGS "-std=gnu99 -Wall -g")
#  include_directories(posix)
#  add_subdirectory(posix)
#  set(plat_libs freertps_posix)
#endif()

#target_link_libraries(freertps ${plat_libs})
#target_link_libraries(freertps metal stm32)

#ament_export_include_directories(include)
#ament_export_libraries(${PROJECT_NAME})

#ament_package(
#  CONFIG_EXTRAS "freertps-extras.cmake"
#)

#install(
#  DIRECTORY cmake
#  DESTINATION share/${PROJECT_NAME}
#)

#install(
#  DIRECTORY include
#  DESTINATION include
#)
