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
