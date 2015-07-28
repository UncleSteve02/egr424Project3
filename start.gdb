file bin/proj3.elf
target remote localhost:3333
monitor reset halt
load
b createThread
c