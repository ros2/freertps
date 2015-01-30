PSM  ?= udp
OS   ?= posix
PLAT ?= generic
TC   ?= 
LIBNAME= freertps_$(PSM)_$(OS)_$(PLAT)
BIN    = bin
LIB    = $(BIN)/lib$(LIBNAME).a
CFLAGS = -Iinclude
LFLAGS = 

COMMONDIR = $(BIN)/common
HALDIR    = $(BIN)/hal/$(PSM)/$(OS)/$(PLAT)
RTPSDIR   = $(BIN)/rtps/$(PSM)

EXAMPLE_NAMES = listener.elf
EXAMPLES = $(addprefix $(BIN)/,$(EXAMPLE_NAMES))

default: $(COMMONDIR) $(HALDIR) $(RTPSDIR) $(LIB) $(EXAMPLES)

$(COMMONDIR):
	mkdir -p $(COMMONDIR)

$(HALDIR):
	mkdir -p $(HALDIR)

$(RTPSDIR):
	mkdir -p $(RTPSDIR)

include common/common.mk
COMMON_SRC_PATHS=$(addprefix common/,$(COMMON_SRCS))
COMMON_OBJS=$(addprefix $(BIN)/,$(COMMON_SRC_PATHS:.c=.o))

include rtps/rtps.mk
include rtps/$(PSM)/psm.mk
RTPS_SRC_PATHS=$(addprefix rtps/,$(RTPS_SRCS))
RTPS_OBJS=$(addprefix $(BIN)/,$(RTPS_SRC_PATHS:.c=.o))

include hal/hal.mk
include hal/$(PSM)/psm.mk
include hal/$(PSM)/$(OS)/os.mk
include hal/$(PSM)/$(OS)/$(PLAT)/plat.mk
HAL_SRC_PATHS=$(addprefix hal/,$(HAL_SRCS))
HAL_OBJS=$(addprefix $(BIN)/,$(HAL_SRC_PATHS:.c=.o))

$(BIN)/%.o: %.c
	$(TC)gcc $(CFLAGS) -c $< -o $@

# will need to fork these into separate libraries at some point to allow
# inclusion of multiple PSMs into the same executable...
$(LIB): $(COMMON_OBJS) $(RTPS_OBJS) $(HAL_OBJS)
	$(TC)ar rcs $@ $(COMMON_OBJS) $(RTPS_OBJS) $(HAL_OBJS)
	$(TC)objdump -S -d $(LIB) > $(BIN)/$(LIBNAME).objdump

$(BIN)/%.elf: examples/%/main.c $(LIB)
	$(TC)gcc $(LFLAGS) -Lbin -Iinclude $< -o $@ -l$(LIBNAME)

.PHONY: clean
clean:
	-rm -rf $(BIN)
