#include "os.h"
#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"

/* Test does not use OS. This is just to test the logic */

int main(void){
    BSP_UART_Init(USB);
    CANbus_Init(CAN_2, NULL, 0);
    Contactors_Init();
    Minions_Init();
    Pedals_Init();
    Task_CommandLine(NULL);
}