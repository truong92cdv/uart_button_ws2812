#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "xparameters.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"

// GPIO definitions
#define BTN_GPIO_BASEADDR    XPAR_AXI_GPIO_BTN_BASEADDR
#define CTRL_GPIO_BASEADDR   XPAR_AXI_GPIO_CTRL_BASEADDR
#define BTN_CHANNEL    1
#define CTRL_CHANNEL   1

// Button masks
#define KEY1_MASK 0x1
#define KEY2_MASK 0x2
#define KEY3_MASK 0x4
#define KEY4_MASK 0x8

// External variables
extern u8 type;
extern u8 color;
extern u8 speed;
extern XGpio GpioBtn, GpioCtrl;
extern XUartPs Uart_PS;

// Function declarations
void send_to_pl(void);
void print_menu(void);
void update_param(char cmd);
void check_buttons(void);
int init_peripherals(void);

#endif // LED_CONTROL_H
