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

    @Store the actual process stack pointer (char*) in r2 from the char ** in r1
    ldr     r2, [r1]

    @Initialize xPSR (first on stack) with 0x01000000 (Thumb bit set)
    movw    r3, 0x0000
    movt    r3, 0x0100
    stmea   r2!, {r3}   @Store xPSR at the highest memory location on the stack

    @Store threadStarter as addressToReturnTo (second on stack)
    ldr     r3, =threadStarter
    stmea   r2!, {r3}   @Store addressToReturnTo (threadStarter) on second highest memory on stack

    @Initialize r0-r3, r12, and r14
    mov     r3, #0
    mov     r12, #0
    stmea   r2!, {r3}           @Simulate storing r14 (LR) as 0
    stmea   r2!, {r3, r12}      @Store r3, r12 (IP) on stack
    stmea   r2!, {r3}           @Simulate storing r2 on the stack by using the value 0
    stmea   r2!, {r3, r12}      @Simulate storing r0 (0) and r1 (0) on stack by storing zeros

    @Save the pointer to the stack pointer
    str r2, [r1]

    @Store r4-r12 and PSP on array
    stm     r0, {r2, r4-r12}

    bx      lr

  /* Save registers in the jump buffer. Their values are
     not important when the thread is first created. This line is the same as
     the first two lines of setjmp(), except we don't save SP and LR since we 
     want to set these to our own values. Really, the only point of this 
     instruction is to advance R0 to the right location in the jump buffer for 
     pointing to SP (without having to do any math :-) */

    stmea  R0!, { R1, R4-R11 }
  
  /* Now we save SP and LR in that order. SP is the R1 parameter, and we have
   * to get the address of threadStarter() into a higher register (so they are
   * placed in the jump buffer in the right order). */

    ldr     R2, .L0
    mov     R3, 0
    stmea   R1!, { R3 }
    stmea   R1!, { R2 }
    stmea   R1!, { R3 }
    stmea   R1!, { R3 }
    stmea   R1!, { R3 }
    stmea   R1!, { R3 }
    stmea   R1!, { R3 }
    stmea   R1!, { R3 }

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
    mrs     R1, PSP
    stm     R0, { R1, R4-R12 }
    bx      lr

    .syntax unified
    .text
    .align 2
    .thumb
    .thumb_func
    .type restoreThreadState,function
    .global restoreThreadState
restoreThreadState:
    ldr R1, [R0]
    add R0, R0
    msr PSP, R1
    ldm R0, { R4-R12 }
    bx lr

