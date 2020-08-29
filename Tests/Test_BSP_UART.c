/**
 * Test file for library to interact with UART
 * 
 * Run this test in conjunction with the simulator
 * GUI. As this generates randomized values, the display
 * will update the values accordingly to show that the
 * display is reading the UART properly
 */

#include "common.h"
#include "config.h"
#include <bsp.h>

#define TX_SIZE 128

int main(void) {
    BSP_UART_Init();
    //while (1) {
        float speed = (rand() % 500) / 10.0;
        int cruiseEn = rand() % 2;
        int cruiseSet = rand() % 2;
        int regenEn = rand() % 2;
        int CANerr = rand() % 10;
        char str[TX_SIZE];
        sprintf(str, "%f, %d, %d, %d, %d", speed, cruiseEn, cruiseSet, regenEn, CANerr);

        float speed1 = (rand() % 500) / 10.0;
        int cruiseEn1 = rand() % 2;
        int cruiseSet1 = rand() % 2;
        int regenEn1 = rand() % 2;
        int CANerr1 = rand() % 10;
        char str1[TX_SIZE];
        sprintf(str1, "%f, %d, %d, %d, %d", speed1, cruiseEn1, cruiseSet1, regenEn1, CANerr1);

        BSP_UART_Write(UART_1, str , TX_SIZE);
        BSP_UART_Write(UART_2, str1, TX_SIZE);
<<<<<<< HEAD

        char out[2][TX_SIZE];
        BSP_UART_Read(UART_1, out[UART_1]);
        BSP_UART_Read(UART_2, out[UART_2]);
        printf("UART 1: %s\tUART 2: %s\n", out[UART_1], out[UART_2]);
    //}
=======
    }
>>>>>>> 2ece6b8454b79418897f1e052f7ffea2324869e2
}
