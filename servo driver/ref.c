// FINAL PROJECT

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FINAL.h"


volatile int dut1; // servo duty cycle
volatile int dut2; // motor duty cycle
void delay();
void initialize_clk();
void intialize_UART(int ibrd, int fbrd);
void initialize_PWM(int dut1, int dut2);
void initialize_PWM_M(int dut1, int dut2);

int main()
{
  initialize_clk(); // setting the system clock
  intialize_UART(104, 11); // setting the UART
  while(1){
    char mode;
    while((UART7_FR & (1 << 4)) != 0){}
    mode = UART7_DR;
    if(mode == 'w') {
      initialize_PWM(0x437F, dut2); // 10.8% (servo middle)
    } else if (mode == 'a'){
      dut1 = dut1 - 0x63f;
      initialize_PWM(0x5a9f, dut2); // 14.5%(servo right)
    }else if (mode == 'd'){
      dut1 = dut1 + 0x63f;
      initialize_PWM(0x2c5f, dut2); // 7.1%(servo left)
    } else if (mode == 's') { // should start from "s", because the motor should be intitiated from minimum
      initialize_PWM(0x437F, 0x251BF);
    } else if (mode == 'q') { // speed up
      dut2 = dut2 - 800;
      initialize_PWM(dut1, dut2);
    } else if (mode == 'e') { // speed down
      dut2 = dut2 + 800;
      initialize_PWM(dut1, dut2);
    } else {
      initialize_PWM(0x437F, 0x251BF); // 10.8%
    }
  }
}

// This function initialize the system clock by configuring the RCC
void initialize_clk() {
  RCC |= (1 << 0); // enable main oscillator
  RCC &= ~(1 << 4); // use the main oscillator as the input source
  RCC &= ~(1 << 5);
  RCC |= (0x3 << 6); // clear XTAL field and configure 16MHz crystal
  RCC |= (1 << 11); // set the bypass to make system clock derived from oscillator source
  RCC &= ~(1 << 22); // do not use system clock divisor
}

// This function describes the behavior for UART#7
void intialize_UART(int ibrd, int fbrd) {
  RCGCUART |= (1 << 7); // enable uart module
  RCGCGPIO |= (1 << 4); // enalbe port E
  GPIOE_AFSEL = (1 << 1) | (1 << 0); // enable alternate function 0-rx 1-tx
  GPIOE_PCTL = (1 << 0) | (1 << 4); // configure PMC1 to tell we are using UART3
  GPIOE_DEN = (1 << 1) | (1 << 0); // enable the digital function
  UART7_CTL &= ~(1 << 0); // disable before configuration
  UART7_IBRD = ibrd; // BRD = 4M / (153600) = 26.0416667
  UART7_FBRD = fbrd; // UARTFBRD = integer(0.0416667 * 64 + 0.5) = 3
  UART7_LCRH = (0x3 << 5); // 8 data bits
  UART7_LCRH &= ~(1 << 3); // 1 stop bit
  UART7_CC = 0x0; // use systme clock
  UART7_CTL = (1 << 0) | (1 << 8) | (1 << 9); // enable UART and its reciver and transmitter
}

// This function describes the behavior for M0PWM0(PB6) and M0PWM1(PB7)
void initialize_PWM(int dut1, int dut2){
  RCGC0 |= (1 << 20); // enable pwm clock
  delay(); // put delay to guarantee RCGC0 finish configuration
  RCGC2 |= (1 << 1); // enable port B
  delay(); // put delay to guarantee RCGC2 finish configuration
  GPIOB_AFSEL |= (1 << 6) | (1 << 7); // enable alternate function for M0PWM0 and M0PWM1
  GPIOB_PCTL &= ~0xFF000000; // clear the PCTL register for GPIO B
  GPIOB_PCTL |= 0x44000000; // enable PMC4 for pin6 and pin7 to use PWM function
  GPIOB_DEN |= 0xC0; // enable the digital function for PB6 and PB7
  RCC |= (1 << 20); // enable the PWM divisor on RCC
  RCC &= ~(1 << 17); // choose the "/2" divisor
  RCC &= ~(1 << 18);
  RCC &= ~(1 << 19);
  PWM0_0_CTL = 0x0; // disable the PWM generator before initialization
  PWM0_0_GENA = 0x0000008C; // setting generator A following datasheet
  PWM0_0_GENB = 0x0000080C; // setting generator B following datasheet
  PWM0_0_LOAD = 0x270FF; // pwm: 8MHz; want: 50Hz // 160000 - 1 tick
  PWM0_0_CMPA = dut1; // set the value of comparator A to determine the duty cycle for M0PWM0
  PWM0_0_CMPB = dut2; // set the value of comparator B to determine the duty cycle for M0PWM1
  PWM0_0_CTL |= (1 << 0); // enable PWM generator
  PWM0_ENABLE = 0x03; // enable output for the PWM generator
}

// This function provides a little delay: larger than 3 ticks of PWM clock
void delay() {
  int clk = 200000;
  while(clk) {
    clk--;
  }
}