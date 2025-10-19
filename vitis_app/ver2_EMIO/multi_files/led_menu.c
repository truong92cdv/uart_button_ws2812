#include "led_control.h"

/* Map speed code -> milliseconds */
static const int speed_table[8] = {200, 250, 300, 350, 400, 450, 500, 550};

/* Draw pretty menu */
void print_menu(void) {
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
void update_param(char cmd) {
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
