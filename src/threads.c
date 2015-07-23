#include <stdio.h>
#include "scheduler.h"
#include "lock.h"

void thread1_UART(void)
{
  while (1) {
    if (lock_acquire(&threadlock)) {
      // Simulate code that is occasionally interrupted
      iprintf("THIS IS T");
          yield(); // context switch "interrupt"
      iprintf("HREAD NU");
          yield(); // context switch "interrupt"
      iprintf("MBER 1\r\n");

      lock_release(&threadlock);
    }
    yield();
  }
}

void thread2_LED(void)
{

}

void thread3_OLED(void)
{

}

void thread4_UART(void)
{
  while (1) {
    if (lock_acquire(&threadlock)) {
      // Simulate code that is occasionally interrupted
      iprintf("this is thread number 2\r\n");

      lock_release(&threadlock);
    }
    yield();
  }
}
