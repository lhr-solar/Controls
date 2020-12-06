#include "bsp.h"
#include "Display.h"

/*
 * Initialize the Wonder Gecko communications.
 */
void Display_Init() {
    BSP_UART_Init(UART_2);
}

/*
 * Updates the Gecko display with the data provided.
 * @param status the new display data
 * @return void
 */
void Display_SetData(display_data_t *status) {
    // Copy the data into a known format
    uint8_t msg[5];
    *((float*)&msg[0]) = status->speed;
    msg[4] = (status->cruiseEnabled << 3)
           | (status->cruiseSet << 2)
           | (status->regenEnabled << 1)
           | (status->canError);

    // Transmit the data
    BSP_UART_Write(UART_2, (char*)msg, sizeof(msg));
}