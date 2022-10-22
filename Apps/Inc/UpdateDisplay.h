/** 
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * @file UpdateDisplay.h
 * @brief Function prototypes for the display application.
 * 
 * This contains function prototypes relevant to the UpdateDisplay
 * application. Call assertUpdateDisplayError after calling any of the
 * functions in this application.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
 * @author Nathaniel Delgado (NathanielDelgado)
*/

#ifndef __UPDATE_DISPLAY_H
#define __UPDATE_DISPLAY_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "Display.h"
#include "Contactors.h"

/**
 * Error types
 */
typedef enum{
	UPDATEDISPLAY_ERR_NONE			= 0,
	UPDATEDISPLAY_ERR_FIFO_PUT		=-1,	// Error putting command in fifo
	UPDATEDISPLAY_ERR_FIFO_POP		=-2,	// Error popping command from fifo
	UPDATEDISPLAY_ERR_PARSE_COMP	=-3,	// Error parsing component/val in SetComponent
} UpdateDisplayError_e;

/**
 * @brief Initializes UpdateDisplay application
 * @returns UpdateDisplayError_e
 */

UpdateDisplayError_e UpdateDisplay_Init();

/**
 * @brief Selects visible page on the display
 * @param page which page to select
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetPage(Page_e page);

/**
 * @brief Sets the state of charge value on the display
 * @param percent charge as a percent
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetSOC(uint8_t percent);

/**
 * @brief Sets the supplemental battery pack voltage value on the display
 * @param mv supplemental battery pack voltage in millivolts
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetSBPV(uint32_t mv);

/**
 * @brief Sets the velocity of the vehicle on the display
 * @param mphTenths velocity of the vehicle in tenths of mph
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetVelocity(uint32_t mphTenths);

/**
 * @brief Sets the accelerator slider value on the display
 * @param percent pressure on accelerator in percent
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetAccel(uint8_t percent);

/**
 * @brief Sets the array indicator state on the display
 * @param state on or off
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetArray(bool state);

/**
 * @brief Sets the motor indicator state on the display
 * @param state on or off
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetMotor(bool state);

/**
 * @brief Sets the gear selection state on the display
 * @param gear DISABLED=N, ENABLED=F, ACTIVE=R
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetGear(TriState_e gear);

/**
 * @brief Sets the regenerative braking indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetRegenState(TriState_e state);

/**
 * @brief Sets the cruise control indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_e
 */
UpdateDisplayError_e UpdateDisplay_SetCruiseState(TriState_e state);

#endif