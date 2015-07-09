default: posix

posix:
	mkdir -p build
	cd build && cmake .. && make
	ln -sf build/examples/listener .

stm32:
	mkdir -p build.stm32
	cd build.stm32 && cmake -Dstm32=ON .. && make

.PHONY: posix stm32 clean
clean:
	-rm -rf build build.stm32

OPENOCD=/usr/local/bin/openocd -f stm32/openocd/stlink-v2-1.cfg -f stm32/openocd/stm32f7-disco.cfg 
#7-disco.cfg
IMAGE=build.stm32/examples/listener.bin
IMAGE_START=0x08000000

program:
	$(OPENOCD) -c "init; sleep 100; halt; sleep 100; flash write_image erase $(IMAGE) $(IMAGE_START); verify_image $(IMAGE) $(IMAGE_START); sleep 100; reset run; sleep 100; shutdown"

dump_flash:
	$(OPENOCD) -c "init; halt; flash banks; dump_image dump.bin $(IMAGE_START) 0x1000; reset run; shutdown"

gdb_server:
	$(OPENOCD) -c "init; halt"

gdb:
	arm-none-eabi-gdb build.stm32/examples/listener.elf -x stm32/openocd/gdb_init_commands

reset:
	$(OPENOCD) -c "init; sleep 100; halt; sleep 100; reset run; sleep 100; shutdown"
