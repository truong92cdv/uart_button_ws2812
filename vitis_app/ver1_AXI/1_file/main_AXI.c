#include "xparameters.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"

#define BTN_GPIO_BASEADDR    XPAR_AXI_GPIO_BTN_BASEADDR
#define CTRL_GPIO_BASEADDR   XPAR_AXI_GPIO_CTRL_BASEADDR
#define BTN_CHANNEL    1
#define CTRL_CHANNEL   1

// WS2812 control signals
static u8 type  = 1;   // 01 = Running
static u8 color = 2;   // 010 = Blue
static u8 speed = 4;   // 100 = 400 ms (default)

// Buttons: KEY1–KEY4 (active-low)
#define KEY1_MASK 0x1
#define KEY2_MASK 0x2
#define KEY3_MASK 0x4
#define KEY4_MASK 0x8

// Instances
static XGpio GpioBtn, GpioCtrl;
static XUartPs Uart_PS;

/* Map speed code -> milliseconds */
static const int speed_table[8] = {200, 250, 300, 350, 400, 450, 500, 550};

/* Write control word to PL */
static void send_to_pl(void) {
    // pack: [2:0] speed | [5:3] color | [7:6] type
    u32 value = (type & 0x3) | ((color & 0x7) << 2) | ((speed & 0x7) << 5);
    XGpio_DiscreteWrite(&GpioCtrl, CTRL_CHANNEL, value);
}

/* Draw pretty menu */
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
        "\x1b[31m",  // Red
        "\x1b[32m",  // Green  
        "\x1b[34m",  // Blue
        "\x1b[33m",  // Yellow
        "\x1b[36m",  // Cyan
        "\x1b[35m",  // Magenta
        "\x1b[37m",  // White
        "\x1b[30m"   // Off (Black)
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

/* Update a parameter from UART command */
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

/* Poll buttons (active-low, simple debounce) */
static void check_buttons(void) {
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
static int init_peripherals(void) {
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
