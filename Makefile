TC   ?= 
LIBNAME= freertps
BIN    = bin
LIB    = $(BIN)/lib$(LIBNAME).a
CFLAGS = -Iinclude
LFLAGS = 

SRCS = freertps.c sdp.c udp.c udp_linux.c
OBJS = $(addprefix $(BIN)/,$(SRCS:.c=.o))

EXAMPLE_NAMES = listener
EXAMPLES = $(addprefix $(BIN)/,$(EXAMPLE_NAMES))

default: $(BIN) $(LIB) $(EXAMPLES)

$(BIN):
	mkdir -p $(BIN)

$(LIB): $(OBJS)

$(BIN)/%.o: %.c
	$(TC)gcc $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(TC)ar rcs $@ $(OBJS)
	@#$(TC)objdump -S -d $(LIB) > $(BIN)/$(LIBNAME).objdump

$(BIN)/%: examples/%.c $(LIB)
	$(TC)gcc $(LFLAGS) -Lbin -Iinclude $< -o $@ -l$(LIBNAME)

.PHONY: clean
clean:
	-rm -rf $(BIN)
