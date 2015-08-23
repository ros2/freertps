include(${PROJECT_SOURCE_DIR}/systems/stm32_common/cmake/stm32_common.cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -mthumb")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FPU_FLAGS} -mcpu=cortex-m4 -T ${PROJECT_SOURCE_DIR}/systems/stm32_common/ld/stm32f4xx.ld")
