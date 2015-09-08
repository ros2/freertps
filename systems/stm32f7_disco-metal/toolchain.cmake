set(STM32_CHIP_HEADER "stm32f746xx.h")
include_directories(${PROJECT_SOURCE_DIR}/systems/metal_common)
include(${PROJECT_SOURCE_DIR}/systems/stm32_common/cmake/stm32f7.cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHSE_VALUE=25000000")
