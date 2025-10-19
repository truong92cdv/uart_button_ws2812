#include "led_control.h"
#include "led_menu.c"
#include "gpio_control.c"


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
