#include <stdio.h>
#include "rit128x96x4.h"
#include "scheduler.h"
#include "lock.h"

// BIT BANDING - Alias address of GPIO-PORTF_DATA_R
#define LED_BB *((volatile unsigned int *) 0x424A7F80)

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
  volatile unsigned ulLoop;
  while (1) {
    // Delay for a bit.
    for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}
    // Toggle LED
    LED_BB ^= 1;
  }
}

void thread3_OLED(void)
{
  volatile unsigned ulLoop;
  while (1) {
    // Clear OLED screen
    RIT128x96x4Clear();
    // Delay for a bit.
    for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}
    RIT128x96x4StringDraw("hello", 5, 50, 5);
    for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}
    yield();
  }
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
