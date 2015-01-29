PSM  ?= udp
OS   ?= posix
PLAT ?= generic
TC   ?= 
LIBNAME= freertps_$(PSM)_$(OS)_$(PLAT)
BIN    = bin
LIB    = $(BIN)/lib$(LIBNAME).a
CFLAGS = -Iinclude
HALDIR = $(BIN)/hal/$(PSM)/$(OS)/$(PLAT)
EXAMPLE_NAMES = listener.elf
EXAMPLES = $(addprefix $(BIN)/,$(EXAMPLE_NAMES))

default: $(HALDIR) $(LIB) $(EXAMPLES)

$(HALDIR):
	mkdir -p $(HALDIR)

#include rtps/rtps.mk
#RTPS_OBJS=$(RTPS_SRCS:%c=$(BINDIR)/%.o)

include hal/hal.mk
include hal/$(PSM)/psm.mk
include hal/$(PSM)/$(OS)/os.mk
include hal/$(PSM)/$(OS)/$(PLAT)/plat.mk
HAL_SRC_PATHS=$(addprefix hal/$(PSM)/$(OS)/$(PLAT)/,$(HAL_SRCS))
HAL_OBJS=$(addprefix $(BIN)/,$(HAL_SRC_PATHS:.c=.o))

$(BIN)/%.o: %.c
	$(TC)gcc $(CFLAGS) -c $< -o $@

$(LIB): $(RTPS_OBJS) $(HAL_OBJS)
	$(TC)ar rcs $@ $(HAL_OBJS)
	$(TC)objdump -S -d $(LIB) > $(BIN)/$(LIBNAME).objdump

$(BIN)/%.elf: examples/%/main.c
	$(TC)gcc $(LFLAGS) -Lbin -l$(LIBNAME) -Iinclude $< -o $@

.PHONY: clean
clean:
	-rm -rf $(BIN)
