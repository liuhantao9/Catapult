#include "Init.h"
#include "port_f.h"
#include "port_e.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "FINAL.h"
#include "SSD2119.h"

void port_f_init();
void port_e_init();

int main()
{ 
  LCD_Init();
  Touch_Init();
  initialize_clk();
  port_f_init();
  intialize_UART(104, 11);
    
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
      FILE *f = fopen("data.txt", "w");
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

void port_f_init() {
    RCGCGPIO |= RCGCGPIO_F_ON;
    GPIO_LOCK = GPIO_CR_UNLOCK;
    GPIO_CR = SWITCH;
    GPIO_UR = SWITCH;
    GPIO_F_DIR = RGB & ~SWITCH;
    GPIO_F_DEN = RGB | SWITCH;
    GPIO_F_DATA = CLEAR;
}

void port_e_init() {
    RCGCGPIO |= RCGCGPIO_E_ON;
    GPIO_E_DIR &= ~PE3;
    GPIO_E_AFSEL |= PE3;
    GPIO_E_DEN &= ~PE3;
    GPIO_E_AMSEL |= PE3;
}
