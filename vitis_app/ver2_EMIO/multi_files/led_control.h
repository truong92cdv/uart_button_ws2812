#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "xparameters.h"
#include "xgpiops.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"

// GPIO definitions
#define EMIO_BASE 78   // EMIO pin base index in PS GPIO
#define CTRL_COUNT 8
#define BTN_COUNT  4

// Button masks
#define KEY1_MASK 0x1
#define KEY2_MASK 0x2
#define KEY3_MASK 0x4
#define KEY4_MASK 0x8

// External variables
extern u8 type;
extern u8 color;
extern u8 speed;
extern XGpioPs psGpio;
extern XUartPs Uart_PS;

// Function declarations
void send_to_pl(void);
void print_menu(void);
void update_param(char cmd);
void check_buttons(void);
int init_peripherals(void);

#endif // LED_CONTROL_H
