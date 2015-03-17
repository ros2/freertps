TC   ?= 
LIBNAME= freertps
BIN    = bin
LIB    = $(BIN)/lib$(LIBNAME).a
# todo: figure out the right flags for other OS to get the posix includes right
CFLAGS = -Iinclude --std=c99 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
LFLAGS = 

SRCS = freertps.c spdp.c udp.c udp_posix.c discovery.c sedp.c
INCS = include/freertps/udp.h include/freertps/spdp.h include/freertps/freertps.h
OBJS = $(addprefix $(BIN)/,$(SRCS:.c=.o))

EXAMPLE_NAMES = listener
EXAMPLES = $(addprefix $(BIN)/,$(EXAMPLE_NAMES))

default: $(BIN) $(LIB) $(EXAMPLES)

$(BIN):
	mkdir -p $(BIN)

$(LIB): $(OBJS)

$(BIN)/%.o: %.c $(INCS)
	$(TC)gcc $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(TC)ar rcs $@ $(OBJS)
	@#$(TC)objdump -S -d $(LIB) > $(BIN)/$(LIBNAME).objdump

$(BIN)/%: examples/%.c $(LIB)
	$(TC)gcc $(LFLAGS) -Lbin -Iinclude $< -o $@ -l$(LIBNAME)

.PHONY: clean
clean:
	-rm -rf $(BIN)
