/*
 * C Program to Count the Number of Trailing Zeroes in Integer
 */
#include "common.h"
#include "config.h"
#include <bsp.h>
 
int main()
{
    BSP_UART_Init(UART_2);

    int count = 0;
    unsigned int num;
 
    num = 176;
    while (num != 0)
    {
        if ((num & 1) == 1)
        {
            break;
        }
        else
        {
            count++;
            num = num >> 1;
 
        }
    }
    printf("\n%d", count);

    while(1);

    return 0;
}