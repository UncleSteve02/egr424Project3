# Modify as appropriate
STELLARISWARE=C:/StellarisWare

CC=arm-none-eabi-gcc -Wall -Os -march=armv7-m -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd -Wl,--gc-sections
	
all: bin/proj3.elf bin/proj3.bin

bin/proj3.elf: src/proj3.c src/syscalls.c src/startup_gcc.c src/threads.c src/create.S src/rit128x96x4.c
	${CC} -o $@ -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc -Tsrc/linkscript.x -Wl,-Map,bin/proj3.map -Wl,--entry,ResetISR src/proj3.c src/startup_gcc.c src/syscalls.c src/rit128x96x4.c src/create.S src/threads.c -ldriver

bin/proj3.bin: bin/proj3.elf
	arm-none-eabi-objcopy -O binary bin/proj3.elf bin/proj3.bin

.PHONY: clean
clean:
	rm -f bin/*.elf bin/*.bin bin/*.map

# vim: noexpandtab  
