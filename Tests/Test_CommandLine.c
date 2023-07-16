#include "os.h"
#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"

/* Test does not use OS. This is just to test the logic */

int main(void){
    BSP_UART_Init(UART_2);
    CANbus_Init(CAN_1, NULL, 0);
    Contactors_Init();
    Minion_init();
    Pedals_Init();
    Task_CommandLine(NULL);
}