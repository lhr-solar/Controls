/* Copyright (c) 2020 UT Longhorn Racing Solar */

/* Driver for the display board. This communicates
 * with the Nextion display over UART in order to show
 * critical information to the driver.
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

/*
 * Initialize the Nextion display
 */
void Display_Init();

/**
 * Value setting subroutines
 */
ErrorStatus Display_SetVelocity(float vel);
ErrorStatus Display_SetMainView(void);


#endif
