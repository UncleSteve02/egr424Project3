/***********************************************
 * Name: Abbi Fair and Steven Demers
 * Date: 7/30/15
 * Course: EGR 424
 * Instructor: Professor Parikh
 * Assignment: Project 3 - Kernel
 **********************************************/

#include <stdio.h>
#include "rit128x96x4.h"
#include "scheduler.h"

// BIT BANDING - Alias address of GPIO-PORTF_DATA_R
#define LED_BB *((volatile unsigned int *) 0x424A7F80)

// Thread (Process) 1.
// This thread is in charge of displaying (using UART):
// "THIS IS THREAD NUMBER 1"
// The first thing this process will check is if it is locked
// by the other UART process. If it isn't, this process will
// print out three parts of the previous phrase with a couple
// yield() calls in between to give the other UART a chance to
// display its phrase. This ultimately displays that the locking
// system that is implemented indeed works.
void thread1_UART(void)
{
  while (1) {
    if (lock_acquire(&threadlock)) {
      // Simulate code that is occasionally interrupted
      iprintf("THIS IS T");
      yield();
      iprintf("HREAD NU");
      yield();
      iprintf("MBER 1\r\n");

      // Releases the lock so thread4_UART can print
      lock_release(&threadlock);
    }
    yield();
  }
}

// Thread (Process) 2
// This thread is in charge of blinking an the on-board LED.
// This LED is located on port F on the MCU. It will count
// up to 50000 cycles to simulate a delay to actually give
// the user a chance to see that is it turning on and off.
// This is called at least every 4ms.
void thread2_LED(void)
{
  volatile unsigned ulLoop;
  while (1) {
    // Delay for a bit.
    for(ulLoop = 0; ulLoop < 50000; ulLoop++) {}
    // Toggle LED
    LED_BB ^= 1;
  }
}

// Thread (Process) 3
// This thread is in charge of displaying an "==" on the OLED.
// This process will print two equal from the 0th position all
// the way to the 104th position on the OLED (horizontally).
// It has two spaces appened to the front and end of the two 
// equal signs so that a RIT128x96x4Clear() function does not
// have to be called (it disrupts the flow of the OLED). This
// way it will put a space in the previous spot where an equal
// sign was. This is more efficient than clearing the whole
// screen.
void thread3_OLED(void)
{
  volatile unsigned ulLoop;
  while (1) {
    // Draw from the left of the screen to the right
    for(ulLoop = 0; ulLoop < 104; ulLoop++) {
      RIT128x96x4StringDraw(" == ", ulLoop, 50, 15);      
    }
    // Draw from the right of the screen to the left
    for(ulLoop = 104; ulLoop > 0; ulLoop--) {
      RIT128x96x4StringDraw(" == ", ulLoop, 50, 15);      
    }
    yield();
  }
}

// Thread (Process) 4
// This thread is in charge of displaying (using UART):
// "this is thread number 2"
// The first thing this process will check is if it is locked
// by the other UART process. If it isn't, this process will
// print out the previous phrase. It very well might get 
// interrupted by the SysTick Timer, but it won't matter because
// the locking system prevents other threads (like thread1_UART)
// to use the UART.
void thread4_UART(void)
{
  while (1) {
    if (lock_acquire(&threadlock)) {
      // Prints a longer phrase than thread1_UART.
      // This gives it a chance to get naturally interrupted 
      // by the SysTick Timer.
      iprintf("this is thread number 2\r\n");

      // Releases the lock so thread4_UART can print
      lock_release(&threadlock);
    }
    yield();
  }
}