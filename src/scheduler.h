#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct {
  int active;       // non-zero means thread is allowed to run
  char *stack;      // pointer to TOP of stack (highest memory location)
  int state[40]; // saved state of thread on a 10 element array
} threadStruct_t;

extern unsigned currThread;
extern unsigned threadlock;
extern void lock_init(unsigned *lock);
extern unsigned lock_acquire(unsigned *lock);
extern void lock_release(unsigned *lock);

void initializeThreads(void);
void yield(void);
void scheduler(void);

#endif // _SCHEDULER_H_
