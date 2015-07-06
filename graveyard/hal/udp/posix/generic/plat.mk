HAL_SRC_NAMES = hal_udp.c 
#sdp.c
HAL_SRCS += $(addprefix udp/posix/generic/,$(HAL_SRC_NAMES))
