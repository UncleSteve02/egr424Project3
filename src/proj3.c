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


void initializeSysTickTimer(void)
{
  NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = 8000; // 1 ms time interval
  NVIC_ST_CURRENT_R = 0; //clear the current count
  NVIC_ST_CTRL_R = 0x00000007; //Enable the timer and clock source and interrupts
  SysTickIntEnable(); //Enable the SysTick Interrupt
  SysTickEnable(); //Enable the SysTick
}

void initializeOLED(void){
  //
  // OLED
  //
  // Initialize the OLED display and write status.
  RIT128x96x4Init(1000000);
  RIT128x96x4StringDraw("Project 3", 20,  0, 15);

}

void initializeLED(void){
  //
  // LED
  //
  // Enable the GPIO port that is used for the on-board LED.
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

  // Enable the GPIO pin for the LED (PF0).  Set the direction as output, and
  // enable the GPIO pin for digital function.
  GPIO_PORTF_DIR_R = 0x01;
  GPIO_PORTF_DEN_R = 0x01;
}

void initializeUART(void){

  //
  // UART
  //
  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  // Set GPIO A0 and A1 as UART pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);



  //CONTEXT SWITCH TIMING SETUP
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  //GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1);



  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));

}


void main(void)
{
  //
  // CLOCK
  //
  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);

  //initialize all peripherals

  initializeLED();
  initializeOLED();
  initializeUART();

  initializeThreads();
  IntMasterEnable();
  initializeSysTickTimer();

  yield();

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
