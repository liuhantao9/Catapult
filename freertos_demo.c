//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "led_task.h"
#include "switch_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "Init.h"

//#include "FINAL.h"
#include "SSD2119.h"

int Transition_Count = -1;
int Switch_Count = -1;
int stop = 0;
int state = 0;
int pass = 0;

#define SYSCTL_RCGC2_R          (*((volatile uint32_t *)0x400FE108))
#define GPIO_PORTD_PCTL_R       (*((volatile uint32_t *)0x4000752C))
#define GPIO_PORTD_AMSEL_R      (*((volatile uint32_t *)0x40007528))
#define GPIO_PORTD_DIR_R        (*((volatile uint32_t *)0x40007400))
#define GPIO_PORTD_AFSEL_R      (*((volatile uint32_t *)0x40007420))
#define GPIO_PORTD_DEN_R        (*((volatile uint32_t *)0x4000751C))
  
#define GPIO_PORTD_DATA_R       (*((volatile uint32_t *)0x400073FC))

xSemaphoreHandle g_pUARTSemaphore;

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}
void PortD_Init(void) {
  SYSCTL_RCGC2_R |= 0x8;
  GPIO_PORTD_PCTL_R &= ~0x000000FF;
  GPIO_PORTD_AMSEL_R &= ~0xFF;
  GPIO_PORTD_DIR_R |= 0xE;
  GPIO_PORTD_AFSEL_R &= ~0xFF;
  GPIO_PORTD_DEN_R |= 0xFF;

}
/*
void pass_button(void* p) {
   int count = 0;
   while (1) {
   Touch_ReadY();
   Touch_ReadX();
   int z = Touch_ReadZ2();
   long result = Touch_GetCoords();
   int x = result >> 16;
   LCD_SetCursor(100,0);
   LCD_PrintInteger(x);
   if ((x > 205 && x < 280)) {
     count++; 
   } else {
     count = 0;
     pass = 0; 
   }
   if (count == 20) {
     pass = 1;
     count = 0;
   }
   vTaskDelay(30);
  }
}

void stop_button(void* p) {
   int count = 0;
   while (1) {
   Touch_ReadY();
   Touch_ReadX();
   int z = Touch_ReadZ2();
   long result = Touch_GetCoords();
   int x = result >> 16;
   LCD_SetCursor(200,0);
   LCD_PrintInteger(x);
   if ((x > 100 && x < 160) || (x > 800 && x < 1000)) {
     count++; 
   } else {
     count = 0;
     stop = 0; 
   }
   if (count == 20) {
     stop = 1;
     count = 0;
   } 
   vTaskDelay(30);
  }
}

void state_fsm(void* p) {
  int count = 0;
  int count2 = 0;
  int count3 = 0;
  while (1) {
    while (state == 0) { 
      GPIO_PORTD_DATA_R  = 0x2;
      if (stop) {
        state = 0;
        count = 0;
      } else if (pass) {
        state = 1;
        count = 0;
      } else {
        count++;
      }
      if (count == 50) {
        count = 0;
        state = 2;
      }
      
    vTaskDelay(30);
    }
    while (state == 1) {
      GPIO_PORTD_DATA_R  = 0x4;
      if (stop) {
        state = 0;
        count = 0;
      } else if (pass) {
        state = 1;
        count = 0;
      } else {
        count++;
      }
      if (count == 50) {
        count = 0;
        state = 0;
      }
      
    vTaskDelay(30);
    } 
    while(state == 2) {
      GPIO_PORTD_DATA_R = 0x8;
      if (stop) {
        state = 0;
        count = 0;
      } else if (pass) {
        state = 1;
        count = 0;
      } else {
        count++;
      }
      if (count == 50) {
        count = 0;
        state = 0;
      }
      
    vTaskDelay(30);
    }
    }
}
*/
/*xQueueHandle Global_Queue_Handle = 0;
void task_tx(void* p)
{
    PortD_Init();
    LCD_Init(); 
    Touch_Init();
    LCD_DrawFilledRect(0, 0, 100, 240,((0x55>>3)<<11) | ((0xFF>>2)<<5) | (0x55>>3)); // start button
    LCD_DrawFilledRect(200, 0, 50, 240,  ((0xFF>>3)<<11) | ((0x55>>2)<<5) | (0x55>>3)); // stop button
      
    Touch_ReadY();
      Touch_ReadX();
      Touch_ReadZ1();
      Touch_ReadZ2();
      long result = Touch_GetCoords();
      int x = result >> 16;
      LCD_SetCursor(200,0);
      LCD_PrintInteger(x);
    int i = 0;
    while(1)
    {
        //GPIO_PORTD_DATA_R  = 0x2;
        if(xQueueSend(Global_Queue_Handle, &i, 500)) {
          /*if (x > 210 && x < 300) { // passenger pressed
             press = 1;
          } else if (x < 180) 
            press = 2;
          } else {
            press = 0;
        }
        vTaskDelay(500);
    }
}

void task_rx(void* p)
{
    PortD_Init();
    LCD_Init(); 
    Touch_Init();
    int i = 0;
    while(1)
    {
        if(xQueueReceive(Global_Queue_Handle, &i, 500)) {
      Touch_ReadY();
      Touch_ReadX();
      Touch_ReadZ1();
      Touch_ReadZ2();
      long result = Touch_GetCoords();
      int x = result >> 16;
      LCD_SetCursor(200,0);
      LCD_PrintInteger(x);
          if (x > 210 && x < 300) {
            vTaskDelay(2000);
            GPIO_PORTD_DATA_R  = 0x4;
          } else if (x < 180) {
            vTaskDelay(2000);
            GPIO_PORTD_DATA_R  = 0x2;
          } else {
            while (1) {
              GPIO_PORTD_DATA_R  = 0x2;
              vTaskDelay(3000);
              GPIO_PORTD_DATA_R  = 0x8;
              vTaskDelay(3000);
            }
          }
    }
    }
}
*/

void catapult(void *p) {
  while(1) {
    Touch_ReadX();
    Touch_ReadY();
    long res = Touch_GetCoords();
    long yPos = res & 0xFF;
    long xPos = res >> 16;
    LCD_SetCursor(0, 0);
    LCD_PrintInteger(xPos);
    LCD_PrintChar(' ');
    LCD_PrintInteger(yPos);
    LCD_PrintChar('\n');
    
    if (yPos > 200 && yPos < 800) {
      char coordstr[10];
      coordstr[0] = '<';
      sprintf(coordstr + 1, "%d", (int) xPos);
      coordstr[4] = ' ';
      sprintf(coordstr + 5, "%d", (int) yPos);
      coordstr[8] = '>';
      coordstr[9] = '\0';
      print_string(coordstr);
      FILE *f = fopen("C:\\Users\\andyk\\Desktop\\data.txt", "w");
      if (!f) LCD_PrintString("File not found");
      fputs(coordstr, f);
      fclose(f);
      for (int i = 0; i < 2000000; i++);
      initialize_PWM(0x437f, 0);
      for (int i = 0; i < 2000000; i++);
      initialize_PWM(0x5a9f, 0);
    }
  }
}
//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    //Global_Queue_Handle = xQueueCreate(1, sizeof(int));
    //PortD_Init();
    LCD_Init(); 
    Touch_Init();
    initialize_clk();
    intialize_UART(104, 11);
    /*
    char turn[] = {"TURN"};
    char launch[] = {"LAUNCH"};
    LCD_DrawFilledRect(30, 50, 100, 140,((0x55>>3)<<11) | ((0xFF>>2)<<5) | (0x55>>3)); // start button
    LCD_DrawFilledRect(190, 50, 100, 140,  ((0xFF>>3)<<11) | ((0x55>>2)<<5) | (0x55>>3)); // stop button
    LCD_SetCursor(70, 120);
    LCD_PrintString(turn);
    LCD_SetCursor(220, 120);
    LCD_PrintString(launch);
    
    xTaskCreate(pass_button, (signed char*)"pass_button", 2048, 0, 2, 0);
    xTaskCreate(stop_button, (signed char*)"stop_button", 2048, 0, 2, 0);
    xTaskCreate(state_fsm, (signed char*)"state_fsm", 2048, 0, 1, 0);
    */
    xTaskCreate(catapult, (signed char*)"catapult", 2048, 0, 2, 0);
    vTaskStartScheduler();

    while(1)
    {
    }
}
