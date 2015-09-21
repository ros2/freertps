include_directories(${PROJECT_SOURCE_DIR}/systems/metal_common)
add_library(metal_common stack.c system.c stubs.c bswap.c enet.c udp.c arm_trap.c usb.c delay.c)
