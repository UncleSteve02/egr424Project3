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

// This is the lock variable used by all threads. Interface functions
// for it are:
//      void lock_init(unsigned *threadlockptr);        // You write this
//      unsigned lock_acquire(unsigned *threadlockptr); // Shown in class
//      void lock_release(unsigned *threadlockptr);     // You write this
unsigned threadlock;

typedef struct {
  int active;       // non-zero means thread is allowed to run
  char *stack;      // pointer to TOP of stack (highest memory location)
  unsigned state[40]; // saved state of thread on a 10 element array
} threadStruct_t;

// thread_t is a pointer to function with no parameters and
// no return value...i.e., a user-space thread.
typedef void (*thread_t)(void);

// These are the external user-space threads. In this program, we create
// the threads statically by placing their function addresses in
// threadTable[]. A more realistic kernel will allow dynamic creation
// and termination of threads.
extern void thread1_UART(void);
extern void thread2_LED(void);
extern void thread3_OLED(void);
extern void thread4_UART(void);

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
unsigned currThread;    // The currently active thread

void initializePeriphs(void)
{
  //
  // CLOCK
  //
  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);
  
  //
  // LED
  //
  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

  // Do a dummy read to insert a few cycles after enabling the peripheral.
  volatile unsigned long ulLoop = SYSCTL_RCGC2_R;

  // Enable the GPIO pin for the LED (PF0).  Set the direction as output, and
  // enable the GPIO pin for digital function.
  GPIO_PORTF_DIR_R = 0x01;
  GPIO_PORTF_DEN_R = 0x01;

  // 
  // OLED
  //
  // Initialize the OLED display and write status.
  RIT128x96x4Init(1000000);
  RIT128x96x4StringDraw("Project 3", 20,  0, 15);

  //
  // UART
  //
  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  // Set GPIO A0 and A1 as UART pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));

}

void initializeSysTickTimer(void)
{
  NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = 8000; // 1 ms time interval
  NVIC_ST_CURRENT_R = 0;
  NVIC_ST_CTRL_R = 0x05;
  SysTickIntEnable();
  SysTickEnable();
}

void priv_to_unpriv(void)
{
  asm volatile("MRS R3, CONTROL\n"
                "ORR R3, R3, #1\n"
                "MSR CONTROL, R3\n"
                "ISB");
  asm volatile("MRS R0, CONTROL");
}

void unpriv_to_priv(void)
{
  asm volatile("MRS R3, CONTROL\n"
                "AND R3, R3, 0xFE\n"
                "MSR CONTROL, R3\n"
                "ISB");
  asm volatile("MRS R0, CONTROL");
}

// This function is called from within user thread context. It executes
// a jump back to the scheduler. When the scheduler returns here, it acts
// like a standard function return back to the caller of yield().
void yield(void)
{
  unpriv_to_priv();
  asm volatile("B schedulerHandler");
  priv_to_unpriv();
}

// This is the starting point for all threads. It runs in user thread
// context using the thread-specific stack. The address of this function
// is saved by createThread() in the LR field of the jump buffer so that
// the first time the scheduler() does a longjmp() to the thread, we
// start here.
void threadStarter(void)
{
  IntMasterDisable();
  iprintf("in currThread <%d>\r\n", currThread);
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

// This function is implemented in assembly language. It sets up the
// initial jump-buffer (as would setjmp()) but with our own values
// for the stack (passed to createThread()) and LR (always set to
// threadStarter() for each thread).
extern void createThread(unsigned *state, char **stack);

extern void saveThreadState(unsigned *state);

extern void restoreThreadState(unsigned *state);

// This is the "main loop" of the program.
void schedulerHandler(void)
{
  IntMasterDisable();

  saveThreadState(threads[currThread].state);

  do {
    if (++currThread >= NUM_THREADS) {
      currThread = 0;
    }
  } while (!threads[currThread].active); 

  restoreThreadState(threads[currThread].state);

  IntMasterEnable();
}

void main(void)
{
  unsigned i;

  initializePeriphs();

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
    iprintf("about to call createThread\r\n");
    createThread(threads[i].state, &threads[i].stack);
    iprintf("createThread completed\r\n");
  }

  unsigned ulLoop;
  for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}

  // Initialize the global thread lock
  lock_init(&threadlock);

  iprintf("about to call initializeSysTickTimer\r\n");
  for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}
  initializeSysTickTimer();

  // iprintf("about to call IntMasterEnable\r\n");
  // for(ulLoop = 0; ulLoop < 200000; ulLoop++) {}
  IntMasterEnable();

  // iprintf("about to call schedulerHandler\r\n");

  // Start running coroutines
  while (1) {
    // asm volatile("B schedulerHandler");
  }

  // If scheduler() returns, all coroutines are inactive and we return
  // from main() hence exit() should be called implicitly (according to
  // ANSI C). However, TI's startup_gcc.c code (ResetISR) does not
  // call exit() so we do it manually.
  exit(0);
}

/*
 * Compile with:
 * ${CC} -o lockdemo.elf -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc 
 *     -Tlinkscript.x -Wl,-Map,lockdemo.map -Wl,--entry,ResetISR 
 *     lockdemo.c create.S threads.c startup_gcc.c syscalls.c rit128x96x4.c 
 *     -ldriver
 */
// vim: expandtab ts=2 sw=2 cindent
