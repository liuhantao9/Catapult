// Testing Code for Servo Driver

// dut          duty cyl    angle
// 0x2EE0       7.5%        90degree
// 0x4AF        10%         180degree

#include <stdint.h>
#include "SSD2119.h"
#include "inc/tm4c123gh6pm.h"

void initialize_PWM(int dut1, int dut2);

int main() {
    LCD_Init();
    Touch_Init();
    while (1) {
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
          for (int i = 0; i < 2000000; i++);          // wait for about 2s
          initialize_PWM(0x437f, 0);                  // turn the servo 90 degree cw
          for (int i = 0; i < 2000000; i++);          // wait for about 2s
          initialize_PWM(0x5a9f, 0);                   // turn the servo 180 degree cw
        }
    }
    return 0;
}

// MIPWM0 (PD0, PD1)
void initialize_PWM(int dut1, int dut2){
    //SYSCTL_RCGC0_R |= (1 << 20);  // enable pwm clock
    SYSCTL_RCGCPWM_R |= 0x02;
    for (int i = 0; i < 200000; i++); // put delay to guarantee RCGC0 finish configuration
    SYSCTL_RCGC2_R |= (1 << 3); // enable port D
    for (int i = 0; i < 200000; i++); // put delay to guarantee RCGC2 finish configuration
    GPIO_PORTD_AFSEL_R |= (1 << 0); // enable alternate function for M1PWM0 and M1PWM1
    GPIO_PORTD_PCTL_R &= ~0x000000FF; // clear the PCTL register for GPIO D
    GPIO_PORTD_PCTL_R |= 0x00000055; // enable PMC5 for pin1 and pin1 to use PWM function
    GPIO_PORTD_DEN_R |= 0x03; // enable the digital function for PB6 and PB7
    SYSCTL_RCC_R |= (1 << 20); // enable the PWM divisor on RCC
    SYSCTL_RCC_R &= ~(1 << 17); // choose the "/2" divisor
    SYSCTL_RCC_R &= ~(1 << 18);
    SYSCTL_RCC_R &= ~(1 << 19);
    PWM1_0_CTL_R = 0x0; // disable the PWM generator before initialization
    PWM1_0_GENA_R = 0x0000008C; // setting generator A following datasheet
    //PWM1_0_GENB_R = 0x0000080C; // setting generator B following datasheet
    PWM1_0_LOAD_R = 0x270FF; // pwm: 8MHz; want: 50Hz // 160000 - 1 tick
    PWM1_0_CMPA_R = dut1; // set the value of comparator A to determine the duty cycle for M0PWM0
    //PWM1_0_CMPB_R = dut2; // set the value of comparator B to determine the duty cycle for M0PWM1
    PWM1_0_CTL_R |= (1 << 0); // enable PWM generator
    PWM1_ENABLE_R = 0x03; // enable output for the PWM generator
}