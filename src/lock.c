/***********************************************
 * Name: Abbi Fair and Steven Demers
 * Date: 7/30/15
 * Course: EGR 424
 * Instructor: Professor Parikh
 * Assignment: Project 3 - Kernel
 **********************************************/

#include <stdio.h>

void lock_init(unsigned *lock)
{
  asm volatile ("clrex;"
                "mov r2, #1;"
                "str r2, [r0]"
                );
}

void lock_indicate_failure(void)
{
  asm volatile("clrex;"
               "mov r0, #0;"
               "bx lr"
              );
}

unsigned lock_acquire(unsigned *lock)
{
  asm volatile ("mov r1, #0;"       
                "ldrex r2, [r0];"   //R2 <- lock value
                "cmp r2, r1;"       //Is it already 0? (hence locked?)
                "itt ne;"
                "strexne r2, r1, [r0];" //if not, try to claim it by writing 0
                                        //R2<-0 if successful, 1 if failure
                "cmpne r2, #1;"     //and check success
                "beq lock_indicate_failure;"  //Branch taken if lock was already 0
                "mov r0, #1;" //Indicate success
                "bx lr"
                );
  return 1; // always succeeds
}

void lock_release(unsigned *lock)
{
  asm volatile ("clrex;"
                "mov r2, #1;"
                "str r2, [r0];"
                "bx lr"
                );
}