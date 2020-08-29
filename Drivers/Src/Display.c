#include "bsp.h"
#include "Display.h"

#define GECKO_DATA_LENGTH 25

/*
 * Initialize the Wonder Gecko communications.
 */
void Display_Init() {
    // Initialize the UART comm. channel
    BSP_UART_Init();
}

/*
 * Updates the Gecko display with the data provided.
 */
void Display_SetData(display_data_t *status) {
    // The Gecko is expecting the code in string format
    // TODO: this should be adjusted to something better,
    //       since snprintf and sscanf add latencies on both MCUs
    char msg[GECKO_DATA_LENGTH];
    int rc = snprintf(msg, sizeof(msg), "%f,%d,%d,%d,%d\n",
                      status->speed,
                      status->cruiseEnabled,
                      status->cruiseSet,
                      status->regenEnabled,
                      status->canError);

    // Make sure the string was written correctly
    #ifdef DEBUG
    assert(rc > 0 && rc < GECKO_DATA_LENGTH);
    #endif

    // Send the string of info to the Gecko controller
    BSP_UART_Write(UART_1, msg, rc);
}