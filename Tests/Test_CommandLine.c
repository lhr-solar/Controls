#include "os.h"
#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"

int main(void){
    BSP_UART_Init(UART_2);
    CANbus_Init(CAN_1);
    Contactors_Init();
    Minion_Init();
    Pedals_Init();
    Task_CommandLine(NULL);
}