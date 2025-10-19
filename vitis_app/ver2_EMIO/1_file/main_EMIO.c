#include "xparameters.h"
#include "xgpiops.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"

/* -------------------------------
   GPIO pin mapping (EMIO)
   -------------------------------
   EMIO[0:7]  → control signals (to PL)
   EMIO[8:11] → 4 buttons (from PL)
   → EMIO GPIO starts from pin #78 on Zynq PS
   so full pin numbers = 78..85 for control, 86..89 for buttons
--------------------------------- */
#define EMIO_BASE 78   // EMIO pin base index in PS GPIO
#define CTRL_COUNT 8
#define BTN_COUNT  4

// WS2812 control parameters
static u8 type  = 1;   // 01 = Running
static u8 color = 2;   // 010 = Blue
static u8 speed = 4;   // 100 = 400 ms (default)

// Buttons mask (KEY1–KEY4 active-low)
#define KEY1_MASK 0x1
#define KEY2_MASK 0x2
#define KEY3_MASK 0x4
#define KEY4_MASK 0x8

// Instances
static XGpioPs psGpio;
static XUartPs Uart_PS;

/* Map speed code -> milliseconds */
static const int speed_table[8] = {200, 250, 300, 350, 400, 450, 500, 550};

/* -----------------------------------------------
   Helper: write packed control word to PL via EMIO
   ----------------------------------------------- */
static void send_to_pl(void) {
    u32 value = (type & 0x3) | ((color & 0x7) << 2) | ((speed & 0x7) << 5);

    for (int i = 0; i < CTRL_COUNT; i++) {
        int pin = EMIO_BASE + i; // EMIO[0]..[7]
        XGpioPs_WritePin(&psGpio, pin, (value >> i) & 1);
    }
}

/* -----------------------------------------------
   Pretty console menu
   ----------------------------------------------- */
static void print_menu(void) {
    xil_printf("\033[2J\033[H"); // clear screen & home cursor
    xil_printf("========================================\r\n");
    xil_printf("     \x1b[1;36mWS2812 LED CONTROL CONSOLE\x1b[0m\r\n");
    xil_printf("========================================\r\n");
    xil_printf("Type  : %02d ", type);
    switch (type) {
        case 0: xil_printf("(All ON)\r\n"); break;
        case 1: xil_printf("(Running)\r\n"); break;
        case 2: xil_printf("(Ping-Pong)\r\n"); break;
        case 3: xil_printf("(Blink)\r\n"); break;
    }
    xil_printf("Color : %03b ", color);
    const char *clr[] = {"Red","Green","Blue","Yellow","Cyan","Magenta","White","Off"};
    const char *color_codes[] = {
        "\x1b[31m","\x1b[32m","\x1b[34m","\x1b[33m",
        "\x1b[36m","\x1b[35m","\x1b[37m","\x1b[30m"
    };
    xil_printf("  (%s%s\x1b[0m)\r\n", color_codes[color], clr[color]);
    xil_printf("Speed :  %d (%d ms)\r\n", speed, speed_table[speed]);
    xil_printf("----------------------------------------\r\n");
    xil_printf("Commands:\r\n");
    xil_printf("  \x1b[33mt\x1b[0m - Change Type\r\n");
    xil_printf("  \x1b[33mc\x1b[0m - Change Color\r\n");
    xil_printf("  \x1b[33m+\x1b[0m - Faster  (-50 ms)\r\n");
    xil_printf("  \x1b[33m-\x1b[0m - Slower  (+50 ms)\r\n");
    xil_printf("----------------------------------------\r\n");
    xil_printf("Enter command: ");
}

/* -----------------------------------------------
   Handle UART command
   ----------------------------------------------- */
static void update_param(char cmd) {
    switch (cmd) {
        case 't': case 'T':
            type = (type + 1) & 0x3; break;
        case 'c': case 'C':
            color = (color + 1) & 0x7; break;
        case '+':
            if (speed > 0) speed--; break;
        case '-':
            if (speed < 7) speed++; break;
        default:
            return;
    }
    send_to_pl();
    print_menu();
}

/* -----------------------------------------------
   Read buttons (active-low)
   ----------------------------------------------- */
static void check_buttons(void) {
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
static int init_peripherals(void) {
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

/* -----------------------------------------------
   Main loop
   ----------------------------------------------- */
int main(void) {
    init_peripherals();
    send_to_pl();
    print_menu();

    while (1) {
        if (XUartPs_IsReceiveData(Uart_PS.Config.BaseAddress)) {
            char c = XUartPs_ReadReg(Uart_PS.Config.BaseAddress, XUARTPS_FIFO_OFFSET);
            update_param(c);
        }
        check_buttons();
    }
}
