#include "led_control.h"

// WS2812 control signals
u8 type  = 1;   // 01 = Running
u8 color = 2;   // 010 = Blue
u8 speed = 4;   // 100 = 400 ms (default)

// Instances
XGpioPs psGpio;
XUartPs Uart_PS;


/* -----------------------------------------------
   Write packed control word to PL via EMIO
   ----------------------------------------------- */
void send_to_pl(void) {
    u32 value = (type & 0x3) | ((color & 0x7) << 2) | ((speed & 0x7) << 5);

    for (int i = 0; i < CTRL_COUNT; i++) {
        int pin = EMIO_BASE + i; // EMIO[0]..[7]
        XGpioPs_WritePin(&psGpio, pin, (value >> i) & 1);
    }
}

/* -----------------------------------------------
   Read buttons (active-low)
   ----------------------------------------------- */
void check_buttons(void) {
    u32 btns = 0;
    for (int i = 0; i < BTN_COUNT; i++) {
        int pin = EMIO_BASE + CTRL_COUNT + i; // EMIO[8..11]
        u32 val = XGpioPs_ReadPin(&psGpio, pin);
        btns |= ((val == 0) ? 1 : 0) << i; // active-low
    }

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
        if (speed > 0) speed--;
        send_to_pl();
        print_menu();
        usleep(200000);
    }
    if (btns & KEY4_MASK) {
        if (speed < 7) speed++;
        send_to_pl();
        print_menu();
        usleep(200000);
    }
}

/* -----------------------------------------------
   Init PS GPIO (EMIO) and UART
   ----------------------------------------------- */
int init_peripherals(void) {
    XGpioPs_Config *cfg = XGpioPs_LookupConfig(0);
    if (!cfg) return XST_FAILURE;
    XGpioPs_CfgInitialize(&psGpio, cfg, cfg->BaseAddr);

    // EMIO GPIO: configure directions
    for (int i = 0; i < CTRL_COUNT; i++) {
        int pin = EMIO_BASE + i;
        XGpioPs_SetDirectionPin(&psGpio, pin, 1); // output
        XGpioPs_SetOutputEnablePin(&psGpio, pin, 1);
    }
    for (int i = 0; i < BTN_COUNT; i++) {
        int pin = EMIO_BASE + CTRL_COUNT + i;
        XGpioPs_SetDirectionPin(&psGpio, pin, 0); // input
    }

    // UART init
    XUartPs_Config *ucfg = XUartPs_LookupConfig(0);
    if (!ucfg) return XST_FAILURE;
    XUartPs_CfgInitialize(&Uart_PS, ucfg, ucfg->BaseAddress);
    XUartPs_SetBaudRate(&Uart_PS, 115200);
    return XST_SUCCESS;
}
