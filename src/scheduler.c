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

#define STACK_SIZE 4096   // Amount of stack space for each thread


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

// This is the lock variable used by all threads. Interface functions
// for it are:
//      void lock_init(unsigned *threadlockptr);        // You write this
//      unsigned lock_acquire(unsigned *threadlockptr); // Shown in class
//      void lock_release(unsigned *threadlockptr);     // You write this
unsigned threadlock;

// thread_t is a pointer to function with no parameters and
// no return value...i.e., a user-space thread.
typedef void (*thread_t)(void);


static thread_t threadTable[] = {
  thread1_UART,
  thread2_LED,
  thread3_OLED,
  thread4_UART
};

#define NUM_THREADS (sizeof(threadTable)/sizeof(threadTable[0]))

// These static global variables are used in scheduler(), in
// the yield() function, and in threadStarter()
static threadStruct_t threads[NUM_THREADS]; // the thread table
unsigned currThread = -1;    // The currently active thread

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

    // After createThread() executes, we can execute a longjmp()
    // to threads[i].state and the thread will begin execution
    // at threadStarter() with its own stack.
    createThread(threads[i].state, (threads[i].stack));
  }
  // Initialize the global thread lock
  lock_init(&threadlock);
}

// This function is called from within user thread context. It executes
// a jump back to the scheduler. When the scheduler returns here, it acts
// like a standard function return back to the caller of yield().
void yield(void)
{
  asm volatile("svc #2");
}

// This is the starting point for all threads. It runs in user thread
// context using the thread-specific stack. The address of this function
// is saved by createThread() in the LR field of the jump buffer so that
// the first time the scheduler() does a longjmp() to the thread, we
// start here.
void threadStarter(void)
{

  //iprintf("in currThread <%d>\r\n", currThread);
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
void scheduler(void)
{

    //Save current thread state
  if(currThread != -1)
    saveThreadState(threads[currThread].state);

  do
  {
    if (++currThread >= NUM_THREADS) {
      currThread = 0;
    }
  } while (threads[currThread].active != 1);

   restoreThreadState(threads[currThread].state);
}