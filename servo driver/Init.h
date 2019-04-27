#include "FINAL.h"
#include <stdint.h>
#include "inc\tm4c123gh6pm.h"

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

char read_char() {
    while (UART7_FR & (1 << 4)); // wait if receiver is empty
    return (char) UART7_DR;
}

void print_char(char c) {
    while (UART7_FR & (1 << 5)); // wait if transmitter is full
    UART7_DR = c;
}

void print_string(char *str) {
    while (*str) {
        print_char(*str);
        str++;
    }
}