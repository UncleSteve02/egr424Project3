#ifndef LOCK_H_
#define LOCK_H_

typedef struct {
	unsigned lock;
	unsigned count;
	int id;
} lock_t;

int getCount(void);
void lock_init(unsigned *);
void lock_indicate_failure(void);
// unsigned lock_acquire(unsigned *, unsigned);
unsigned lock_acquire(unsigned *);
void lock_release(unsigned *);
void __malloc_lock(unsigned *, unsigned);
void __malloc_unlock(unsigned *, unsigned);

#endif