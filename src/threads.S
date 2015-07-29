/*
 * Implement the thread creation task:
 *
 *   - initialize the jump buffer with appropriate values for
 *     R13 (stack) and R14 (first address to jump to)
 *   - all other registers are irrelevant upon thread creation
 *
 *   In the jump buffer, the R13 slot is set to the second parameter of this
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

  push {r4-r9, r10, r11}

  mov r12, #0x0000
  movt r12, #0x0100

  ldr  r11, .L0

  ldr  r10, =#0xFFFFFFFF

  mov r9, #0
  mov r8, #0
  mov r7, #0
  mov r6, #0
  mov r5, #0

  STMFD r1!, {r5-r12}
  stmea r0!, {r1,r4-r12}
  pop {r4-r9, r10, r11}
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
    mrs r1, psp
    stmia r0, {r1,r4-r12}
    bx lr




    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type restoreThreadState,function
    .global restoreThreadState

restoreThreadState:
  ldmia r0, {r1,r4-r12}

  msr psp, r1

  LDR LR, =0xFFFFFFFD
  bx lr

