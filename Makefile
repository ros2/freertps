SYSTEMS ?= native-posix                \
           stm32f7_disco-metal         \
           stm32f4_disco-metal         \
           stm3240g_eval-metal         \
           samv71_xplained_ultra-metal

.PHONY: all clean $(SYSTEMS)
all: utils/bin/console $(SYSTEMS)

BUILT_SYSTEMS:=$(filter-out msgs,$(shell ls build))
BUILT_APPS:=$(foreach SYSTEM, $(BUILT_SYSTEMS), $(foreach APP, $(shell ls build/$(SYSTEM)/apps), $(APP)-$(SYSTEM)))
PROGRAM_TARGETS:=$(foreach APP, $(BUILT_APPS), program-$(APP))
GDB_TARGETS:=$(foreach APP, $(BUILT_APPS), gdb-$(APP))
RESET_TARGETS:=$(foreach APP, $(BUILT_APPS), reset-$(APP))
GDB_SERVER_TARGETS:=$(foreach APP, $(BUILT_APPS), gdb_server-$(APP))

utils/bin/console:
	cd utils && make

$(SYSTEMS): %: build/%
	@echo $@
	cd build/$@ && cmake ../.. -DSYSTEM=$@ -Dfreertps_standalone=ON && make --no-print-directory

build/%:
	mkdir -p $@

clean:
	-rm -rf build

OPENOCD=/usr/local/bin/openocd -f stm32/openocd/stlink-v2-1.cfg -f stm32/openocd/stm32f7-disco.cfg 
IMAGE=build.stm32/examples/listener.bin
IMAGE_START=0x08000000

list-apps:
	@echo $(PROGRAM_TARGETS)

genmsg:
	r2/mega_genmsg.py

.PHONY: $(PROGRAM_TARGETS) $(RESET_TARGETS) $(GDB_SERVER_TARGETS) $(GDB_TARGETS) list-apps genmsg

$(PROGRAM_TARGETS) : 
	scripts/task_runner program $(subst program-,,$@)

$(RESET_TARGETS) :
	scripts/task_runner reset $(subst reset-,,$@)

$(GDB_SERVER_TARGETS) :
	scripts/task_runner gdb_server $(subst gdb_server-,,$@)

$(GDB_TARGETS) :
	# this is really ugly. but we want to run gdb inside Make rather than 
	# through  task_runner (for now, at least) so the console works right
	PROGRAM=$(firstword $(subst -, ,$(subst gdb-,,$@))); SYSTEM=$(word 2,$(subst -, ,$(subst gdb-,,$@))); OS=$(word 3,$(subst -, ,$(subst gdb-,,$@))); echo $$SYSTEM-$$OS; arm-none-eabi-gdb build/$$SYSTEM-$$OS/apps/$$PROGRAM/$$PROGRAM.elf -x systems/stm32_common/openocd/gdb_init_commands
	
# echo $$SYSTEM
	
#$(echo $$SYSTEM
#scripts/task_runner gdb $(subst gdb-,,$@)

#: build/$(firstword 

#%-program:
#	@echo $*

#dump_flash:
#	$(OPENOCD) -c "init; halt; flash banks; dump_image dump.bin $(IMAGE_START) 0x1000; reset run; shutdown"

#gdb_server:
#	$(OPENOCD) -c "init; halt"

#gdb:
#	arm-none-eabi-gdb build.stm32/examples/listener.elf -x stm32/openocd/gdb_init_commands

#reset:
#	$(OPENOCD) -c "init; sleep 100; halt; sleep 100; reset run; sleep 100; shutdown"
