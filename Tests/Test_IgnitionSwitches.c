/*
 *Test for Ignition Switch
 */

#include "common.h"
#include "config.h"
#include "Minions.h"
#include <bsp.h>

#define TX_SIZE 128

int main()
{
    // Minions_Init();

    BSP_UART_Init(UART_3);

    int result1;
    int result2;

    int size;

    while (1)
    {
        // result1 = Switches_Read(IGN_1);
        // result2 = Switches_Read(IGN_2);

        result1 = 1;
        result2 = 0;

        char str[TX_SIZE];
        size = sprintf(str, "IGN_1 = %d, IGN_2 = %d \r\n", result1, result2);
        BSP_UART_Write(UART_3, str, size);
        // for(int i = 0; i < 999999; i++){} //delay for half a second
    }
}