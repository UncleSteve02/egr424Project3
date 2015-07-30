#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/lm3s6965.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "rit128x96x4.h"
#include "scheduler.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"


// This function is implemented in assembly language. It sets up the
// initial jump-buffer (as would setjmp()) but with our own values
// for the stack (passed to createThread()) and LR (always set to
// threadStarter() for each thread).
extern void createThread(int *state, char *stack);
extern void saveThreadState(int *state);
extern void restoreThreadState(int *state);

// These are the external user-space threads. In this program, we create
// the threads statically by placing their function addresses in
// threadTable[]. A more realistic kernel will allow dynamic creation
// and termination of threads.
extern void thread1_UART(void);
extern void thread2_LED(void);
extern void thread3_OLED(void);
extern void thread4_UART(void);

// thread_t is a pointer to function with no parameters and
// no return value...i.e., a user-space thread.
typedef void (*thread_t)(void);

static thread_t threadTable[] = {
  thread1_UART,
  thread2_LED,
  thread3_OLED,
  thread4_UART,
};

#define NUM_THREADS (sizeof(threadTable)/sizeof(threadTable[0]))
#define STACK_SIZE 4096   // Amount of stack space for each thread

// These static global variables are used in scheduler(), in
// the yield() function, and in threadStarter()
static threadStruct_t threads[NUM_THREADS]; // the thread table

// This is the lock variable used by UART threads.
unsigned threadlock;

//active thread right now
//start at -1 for first pass
unsigned currThread = -1;

void initializeThreads(void)
{
  unsigned i;
  // Create all the threads and allocate a stack for each one
  for (i=0; i < NUM_THREADS; i++) {
    // Mark thread as runnable
    threads[i].active = 1;

    // Allocate stack
    threads[i].stack = (char *)malloc(STACK_SIZE) + STACK_SIZE;
    if (threads[i].stack == 0) {
      iprintf("Out of memory\r\n");
      exit(1);
    }
    createThread(threads[i].state, (threads[i].stack));
  }
  // Initialize the global thread lock
  lock_init(&threadlock);
}

// This function is called from within user thread context. It executes
// a SVC interrupt which will be redirected in the scheduler.
void yield(void)
{
    asm volatile("svc #2");
}

// This is the starting point for all threads. It runs in user thread
// context using the thread-specific stack. The address of this function
// is saved by createThread() in the LR field of the array element so that
// the first time the scheduler() finishes with a thread, we start here.
void threadStarter(void)
{

  // Call the entry point for this thread. The next line returns
  // only when the thread exits.
  (*(threadTable[currThread]))();

  // Do thread-specific cleanup tasks. Currently, this just means marking
  // the thread as inactive. Do NOT free the stack here because we're
  // still using it! Remember, this function runs in user thread context.
  threads[currThread].active = 0;

  // This yield returns to the scheduler and never returns back since
  // the scheduler identifies the thread as inactive.
  yield();
}

// This is the "main loop" of the program.
// Accessible by SysTick or SVC interrupts.
void scheduler(void)
{
    //Start Context Switch Time Measurement

  if(currThread != -1)
    saveThreadState(threads[currThread].state);

  currThread++;
  while(!threads[currThread].active){
        if (currThread++ >= NUM_THREADS)
          currThread = 0;
  }

    //Stop Context Switch Time Measurement

   restoreThreadState(threads[currThread].state);
}
