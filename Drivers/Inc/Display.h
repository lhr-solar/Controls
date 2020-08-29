/* Driver for the display board. This communicates
 * with the Wonder Gecko development board over UART
 * to indicate what needs to be displayed to the driver
 * of the vehicle.
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

typedef struct _display_data {
    float speed;
    bool  cruiseEnabled;
    bool  cruiseSet;
    bool  regenEnabled;
    bool  canError;
} display_data_t;

/*
 * Initialize the Wonder Gecko communications.
 */
void Display_Init();

/*
 * Updates the Gecko display with the data provided.
 */
void Display_SetData(display_data_t *status);

#endif