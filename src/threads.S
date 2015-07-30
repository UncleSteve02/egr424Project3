/***********************************************
 * Name: Abbi Fair and Steven Demers
 * Date: 7/30/15
 * Course: EGR 424
 * Instructor: Professor Parikh
 * Assignment: Project 3 - Kernel
 **********************************************
 *
 * Implement the thread creation task:
 *
 *   - initialize the array of elements with appropriate values for
 *     R13 (stack) and R14 (addressToReturnTo)
 *   - all other registers are irrelevant upon thread creation
 *
 *   In the array, the R13 slot is set to the second parameter of this
 *   function (the top-of-stack address, passed in R1). The R14 slot is set to
 *   the address of the threadStarter() function.
 *
 *   The C prototype for this function call is:
 *       createThread(threads[i].state, threads[i].stack)
 *   thus:
 *       R0 <-- state (array of 10 elements for r4-r11, SP)
 *       R1 <-- stack (address of top-of-stack)
 */
    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type createThread,function
    .global createThread
createThread:

  @ Push r4-r11 onto the stack
  push {r4-r11}

  @ Moving 0x01000000 into r12 (will later be xPSR)
  mov r12, #0x0000
  movt r12, #0x0100

  @ Loading threadStarter() function into r11 (will later be A2R2)
  ldr  r11, .L0

  @ Loading 0 into r10 (will later be R14)
  mov r10, 0

  @ Loading 0 into r9 (will later be R12)
  mov r9, 0

  @ Loading 0 into r8 (will later be R3)
  mov r8, 0

  @ Loading 0 into r7 (will later be R2)
  mov r7, 0

  @ Loading 0 into r6 (will later be R1)
  mov r6, 0

  @ Loading 0 into r5 (will later be R0)
  mov r5, 0

  @ Store values r5-r12 onto pointer at r1, incrementing the pointer each time
  stmdb r1!, {r5-r12}

  @ Store values r1 r4-r12 onto pointer at r0, incrementing the pointer each time
  stmia r0!, {r1,r4-r12}

  @ Pop r4-r11 off of the stack
  @pop {r4-r9, r10, r11}
  pop {r4-r11}

  @ Branch to Link Register
  bx      lr


.L0:
    .word   threadStarter





    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type saveThreadState,function
    .global saveThreadState

saveThreadState:
    
  @ Move PSP value to r1
  mrs r1, psp

  @ Stores values from the stack pointer r0 into r1, r4-r12
  stmia r0, {r1, r4-r12}
 
  @ Branch to Link Register
  bx lr




    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type restoreThreadState,function
    .global restoreThreadState

restoreThreadState:

  @ Loading  r1, r4-r12 values onto the stack pointer r0 
  ldmia r0, {r1, r4-r12}

  @ Move value in r1 to PSP
  msr psp, r1

  @ Loading 0xFFFFFFFD into lr
  ldr lr, =0xFFFFFFFD

  @ Branch to Link Register
  bx lr

