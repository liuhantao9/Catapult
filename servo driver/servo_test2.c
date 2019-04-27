/*#include "Init.h"
#include "port_f.h"
#include "port_e.h"
#include <stdint.h>
#include "FINAL.h"

void port_f_init();
void port_e_init();

int main() {
    char c;
    

  initialize_clk();
  port_f_init();
  port_e_init();
  intialize_UART(104, 11);
  char mode;
  while(1) {
    print_string("Enter \"e\" for 80MHz, or \"f\" for 4MHz: \n\r");
    
    c = read_char();
    print_char(c);
    print_string("\n\r");
    mode = UART7_DR;
    print_char(mode);
    if (mode == 'r') {
      GPIO_F_DATA = RED;
    } else if (mode == 'B') {
      GPIO_F_DATA = BLUE;
    } else {
      GPIO_F_DATA = GREEN;
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
*/
/*/ FINAL PROJECT

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FINAL.h"
//#include "Init.h"
#include "port_f.h"
#include "port_e.h"


volatile int dut1; // servo duty cycle
volatile int dut2; // motor duty cycle
void delay();
void initialize_clk();
void intialize_UART(int ibrd, int fbrd);
void initialize_portf();

int main()
{
  initialize_clk(); // setting the system clock
  intialize_UART(104, 11); // setting the UART
  initialize_portf();
  while(1){
    char mode;
    while((UART7_FR & (1 << 4)) != 0){}
    mode = UART7_DR;
    if(mode == 'w') {
      GPIOF_DATA = BLUE;
    } else {
      GPIOF_DATA = GREEN;
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



// This function provides a little delay: larger than 3 ticks of PWM clock
void delay() {
  int clk = 200000;
  while(clk) {
    clk--;
  }
}

void initialize_portf() {
    RCGCGPIO |= RCGCGPIO_F_ON;
    GPIO_LOCK = GPIO_CR_UNLOCK;
    GPIO_CR = SWITCH;
    GPIO_UR = SWITCH;
    GPIOF_DIR = RGB & ~SWITCH;
    GPIOF_DEN = RGB | SWITCH;
    GPIOF_DATA = CLEAR;
}
*/
// Testing Code for Servo Driver

// dut          duty cyl    angle
// 0x2EE0       7.5%        90degree
// 0x4AF        10%         180degree

#include <stdint.h>
#include "final.h"

#define RCGCGPIO       *((volatile unsigned long *) 0x400FE608)
#define GPIO_F_DIR     *((volatile unsigned long *) 0x40025400)
#define GPIO_F_DEN     *((volatile unsigned long *) 0x4002551C)
#define GPIO_F_DATA    *((volatile unsigned long *) 0x400253FC)
#define GPIO_LOCK      *((volatile unsigned long *) 0x40025520)
#define GPIO_CR        *((volatile unsigned long *) 0x40025524)
#define GPIO_UR        *((volatile unsigned long *) 0x40025510)

void initialize_PWM(int dut1, int dut2);
void init_port_f();

int main() {
  init_port_f();
  while (1) {
    int switch_1 = GPIO_F_DATA & 0x10;
    if (!switch_1) {
      for (int i = 0; i < 2000000; i++);          // wait for about 2s
      initialize_PWM(0x437f, 0);                  // turn the servo 90 degree cw
      for (int i = 0; i < 2000000; i++);          // wait for about 2s
      initialize_PWM(0x5a9f, 0);                   // turn the servo 180 degree cw
    }
  }
  return 0;
}

void init_port_f() {
  RCGCGPIO = 0x20;
  GPIO_LOCK = 0x4C4F434B;
  GPIO_CR = 0x11;
  GPIO_UR = 0x11;
  GPIO_F_DEN = 0x0E | 0x11;
  GPIO_F_DIR = 0x0E & ~0x11;
  GPIO_F_DATA = 0x00;
}
  

void initialize_PWM(int dut1, int dut2){
  RCGC0 |= (1 << 20); // enable pwm clock
  for (int i = 0; i < 200000; i++); // put delay to guarantee RCGC0 finish configuration
  RCGC2 |= (1 << 1); // enable port B
  for (int i = 0; i < 200000; i++); // put delay to guarantee RCGC2 finish configuration
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