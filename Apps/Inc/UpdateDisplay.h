/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file UpdateDisplay.h
 * @brief Function prototypes for the display application.
 * 
 * This contains function prototypes relevant to the UpdateDisplay
 * application. Call assertUpdateDisplayError after calling any of the
 * functions in this application.
 * 
 * 
 * @addtogroup UpdateDisplay
 * @{
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
	UPDATEDISPLAY_ERR_NONE,
	UPDATEDISPLAY_ERR_FIFO_PUT,     // Error putting command in fifo
	UPDATEDISPLAY_ERR_FIFO_POP,     // Error popping command from fifo
	UPDATEDISPLAY_ERR_PARSE_COMP,   // Error parsing component/val in SetComponent
	UPDATEDISPLAY_ERR_DRIVER        // Driver call returned an error
} UpdateDisplayError_t;

/**
 * For display elements with three states
 */
typedef enum{
	STATE_0	=0,
	STATE_1	=1,
	STATE_2	=2
} TriState_t;

// For cruise control and regen
#define DISP_DISABLED STATE_0
#define DISP_ENABLED STATE_1	// Able to be used
#define DISP_ACTIVE STATE_2	// Actively being used right now

// For gear changes
#define DISP_NEUTRAL STATE_0
#define DISP_FORWARD STATE_1
#define DISP_REVERSE STATE_2

/**
 * @brief Initializes UpdateDisplay application
 * @returns UpdateDisplayError_t
 */

UpdateDisplayError_t UpdateDisplay_Init();

/**
 * @brief Selects visible page on the display
 * @param page which page to select
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetPage(Page_t page);

/**
 * @brief Sets the state of charge value on the display
 * @param percent charge as a percent (0 digits of precision)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetSOC(uint8_t percent);

/**
 * @brief Sets the supplemental battery pack voltage value on the display
 * @param mv supplemental battery pack voltage in millivolts
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetSBPV(uint32_t mv);

/**
 * @brief Sets the velocity of the vehicle on the display
 * @param mphTenths velocity of the vehicle in tenths of mph (1 digit of precision)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetVelocity(uint32_t mphTenths);

/**
 * @brief Sets the accelerator slider value on the display
 * @param percent pressure on accelerator in percent
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetAccel(uint8_t percent);

/**
 * @brief Sets the array indicator state on the display
 * @param state array contactor on (true) or off (false)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetArray(bool state);

/**
 * @brief Sets the motor contactor indicator state on the display
 * @param state motor contactor on (true) or off (false)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetMotor(bool state);

/**
 * @brief Sets the gear selection state on the display
 * @param gear DISABLED=N, ENABLED=F, ACTIVE=R
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetGear(TriState_t gear);

/**
 * @brief Sets the regenerative braking indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetRegenState(TriState_t state);

/**
 * @brief Sets the cruise control indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError_t UpdateDisplay_SetCruiseState(TriState_t state);

/**
 * @brief Clears the display message queue and sets the message counter semaphore value to 0
 * @param none
 * @returns none
*/
void UpdateDisplay_ClearQueue(void);

#endif

/** @} */
