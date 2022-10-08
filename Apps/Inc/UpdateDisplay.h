/** Copyright (c) 2022 UT Longhorn Racing Solar
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
	UPDATEDISPLAY_ERR_NONE				= 0,
	UPDATEDISPLAY_ERR_FIFO_PUT		=-1,	// Error putting command in fifo
	UPDATEDISPLAY_ERR_FIFO_POP		=-2,	// Error popping command from fifo
	UPDATEDISPLAY_ERR_PARSE_COMP	=-3,	// Error parsing component/val in SetComponent
} UpdateDisplay_Error_t;

/**
 * @brief Error handler for any UpdateDisplay errors. Call this after any display application function.
 */
void assertUpdateDisplayError(UpdateDisplay_Error_t err);

/**
 * @brief Initializes UpdateDisplay application
 * @returns UpdateDisplay_Error_t
 */

UpdateDisplay_Error_t UpdateDisplay_Init();

/**
 * @brief Selects visible page on the display
 * @param page which page to select
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetPage(Page_t page);

/**
 * @brief Sets the state of charge value on the display
 * @param percent charge as a percent
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetSOC(uint8_t percent);

/**
 * @brief Sets the supplemental battery pack voltage value on the display
 * @param mv supplemental battery pack voltage in millivolts
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetSBPV(uint32_t mv);

/**
 * @brief Sets the velocity of the vehicle on the display
 * @param mphTenths velocity of the vehicle in tenths of mph
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetVelocity(uint32_t mphTenths);

/**
 * @brief Sets the accelerator slider value on the display
 * @param percent pressure on accelerator in percent
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetAccel(uint8_t percent);

/**
 * @brief Sets the array indicator state on the display
 * @param state on or off
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetArray(bool state);

/**
 * @brief Sets the motor indicator state on the display
 * @param state on or off
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetMotor(bool state);

/**
 * @brief Sets the gear selection state on the display
 * @param gear DISABLED=N, ENABLED=F, ACTIVE=R
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetGear(TriState_t gear);

/**
 * @brief Sets the regenerative braking indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetRegenState(TriState_t state);

/**
 * @brief Sets the cruise control indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetCruiseState(TriState_t state);

/**
 * @brief Sets the left blinker state on the display
 * @param state blinking or not blinking
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetLeftBlink(bool state);

/**
 * @brief Sets the right blinker state on the display
 * @param state blinking or not blinking
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetRightBlink(bool state);

/**
 * @brief Sets the headlight state on the display
 * @param state on or off
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetHeadlight(bool state);

/**
 * @brief Sets the hazard state on the display
 * @param state on or off
 * @returns UpdateDisplay_Error_t
 */
UpdateDisplay_Error_t UpdateDisplay_SetHazard(bool state);

#endif