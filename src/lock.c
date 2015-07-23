#include <stdio.h>
#include "lock.h"

volatile int count;
volatile unsigned currLockThread;

int getCount(void) {
	return count;
}

void lock_init(unsigned *lock)
{
  asm volatile ("clrex;"
                "mov r2, #1;"
                "str r2, [r0]"
                );
  // count = 0;
  // currLockThread = -1;
}

void lock_indicate_failure(void)
{
  // iprintf("in lock_indicate_failure()\r\n");
  // currLockThread = -1;
  asm volatile("clrex;"
               "mov r0, #0;"
               "bx lr"
              );
}

// unsigned lock_acquire(unsigned *lock, unsigned currThread)
unsigned lock_acquire(unsigned *lock)
{
  // iprintf("in lock_acquire()\r\n");
  // currLockThread = currThread;
  asm volatile ("mov r1, #0;"
                "ldrex r2, [r0];"
                "cmp r2, r1;"
                "itt ne;"
                "strexne r2, r1, [r0];"
                "cmpne r2, #1;"
                "beq lock_indicate_failure;"
                "mov r0, #1;"
                "bx lr"
                );
  return 1; // always succeeds
}

void lock_release(unsigned *lock)
{
  // iprintf("in lock_release()\r\n");
  asm volatile ("clrex;"
                "mov r2, #1;"
                "str r2, [r0];"
                "bx lr"
                );
}

void __malloc_lock(unsigned *lock, unsigned currThread)
{
	if (currThread == currLockThread && *lock == 0) {
	// if (currThread == currLockThread) {
		count++;
	}
}

void __malloc_unlock(unsigned *lock, unsigned currThread)
{
	if (currThread == currLockThread && *lock == 0) {
		count--;
		if (count < 1) {
			currLockThread = -1;
			lock_release(lock);
		}
	}
}
