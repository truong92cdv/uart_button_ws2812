#include "led_control.h"

// WS2812 control signals
u8 type  = 1;   // 01 = Running
u8 color = 2;   // 010 = Blue
u8 speed = 4;   // 100 = 400 ms (default)

// Instances
XGpio GpioBtn, GpioCtrl;
XUartPs Uart_PS;


/* Write control word to PL */
void send_to_pl(void) {
    // pack: [2:0] speed | [5:3] color | [7:6] type
    u32 value = (type & 0x3) | ((color & 0x7) << 2) | ((speed & 0x7) << 5);
    XGpio_DiscreteWrite(&GpioCtrl, CTRL_CHANNEL, value);
}

/* Poll buttons (active-low, simple debounce) */
void check_buttons(void) {
    u32 btns = XGpio_DiscreteRead(&GpioBtn, BTN_CHANNEL);
    btns = (~btns) & 0xF;

    if (btns & KEY1_MASK) { 
        type = (type + 1) & 0x3; 
        send_to_pl();
        print_menu();
        usleep(200000); 
    }
    if (btns & KEY2_MASK) { 
        color = (color + 1) & 0x7; 
        send_to_pl();
        print_menu();
        usleep(200000); 
    }
    if (btns & KEY3_MASK) { 
        if (speed > 0) {
            speed--; 
            send_to_pl();
            print_menu();
        }
        usleep(200000); 
    }
    if (btns & KEY4_MASK) { 
        if (speed < 7) {
            speed++; 
            send_to_pl();
            print_menu();
        }
        usleep(200000); 
    }
}

/* System init */
int init_peripherals(void) {
    XGpio_Initialize(&GpioBtn, BTN_GPIO_BASEADDR);
    XGpio_Initialize(&GpioCtrl, CTRL_GPIO_BASEADDR);
    XGpio_SetDataDirection(&GpioBtn, BTN_CHANNEL, 0xFFFFFFFF);
    XGpio_SetDataDirection(&GpioCtrl, CTRL_CHANNEL, 0x00000000);

    XUartPs_Config *cfg = XUartPs_LookupConfig(0);
    if (!cfg) return XST_FAILURE;
    XUartPs_CfgInitialize(&Uart_PS, cfg, cfg->BaseAddress);
    XUartPs_SetBaudRate(&Uart_PS, 115200);
    return XST_SUCCESS;
}
