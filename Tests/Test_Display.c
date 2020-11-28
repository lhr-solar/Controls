/**
 * Test file for library to interact with gecko display
 * 
 * This test will send random values to the gecko and 
 * then inspect the UART channel through which the message
 * is suposed to be sent. Ideally, this test should be run 
 * with a gui simulator for the display (whenever that gets written)
 */


#include "common.h"
#include "config.h"
#include "Display.h"
#include <bsp.h>

#define TX_SIZE 128

int main() {
    Display_Init();
    display_data_t packet;
    char in[TX_SIZE];
    while(1) {
        packet.speed = (rand() % 500) / 10.0;
        packet.cruiseEnabled = rand() % 2;
        packet.cruiseSet = rand() % 2;
        packet.regenEnabled = rand() % 2;
        packet.canError = rand() % 10;

        Display_SetData(&packet);

        // We have no way of showing what's on the screen right now
        // but we send the data to the gecko over UART_1 (probably)
        // So we'll just read from the UART for debugging purposes

        BSP_UART_Read(UART_2, in);
        in[strlen(in)-1] = '\0'; // remove the newline
        printf("Gecko Data: %s\r", in);
    }
}
