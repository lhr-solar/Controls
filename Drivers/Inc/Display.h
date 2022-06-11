/* Copyright (c) 2020 UT Longhorn Racing Solar */

/* Driver for the display board. This communicates
 * with the Nextion display over UART in order to show
 * critical information to the driver.
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"
#include "config.h"

/*
 * Initialize the Nextion display
 */
void Display_Init();

/**
 * Value setting subroutines
 */
ErrorStatus Display_SetMainView(void);
ErrorStatus Display_SetPrechargeView(void);

ErrorStatus Display_CruiseEnable(State on);
ErrorStatus Display_CruiseSet(State on);
ErrorStatus Display_SetVelocity(float vel);
ErrorStatus Display_SetSBPV(uint16_t mv);
ErrorStatus Display_SetError(int idx, char *err);
ErrorStatus Display_NoErrors(void);

/**
 * @brief Update the charge state of the battery onto the display.
 * @param chargeState 
 * @return void 
 */
ErrorStatus Display_SetChargeState(uint32_t chargeState);

/**
 * @brief Updates the display with whether regenerative braking / charging is allowed or not
 * @param ChargeEnabled a state value indicating whether or not charging is enabled
 * @return void
 */
ErrorStatus Display_SetRegenEnabled(State ChargeEnabled);

#endif
