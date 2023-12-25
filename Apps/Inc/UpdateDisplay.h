/**
 * @file UpdateDisplay.h
 * @brief Function prototypes for the display application.
 * 
 * This contains function prototypes relevant to the UpdateDisplay
 * application. Call assertUpdateDisplayError after calling any of the
 * functions in this application.
 * 
 */

#ifndef __UPDATE_DISPLAY_H
#define __UPDATE_DISPLAY_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "Display.h"
#include "Contactors.h"

/**
 * @enum UpdateDisplayError_t
 * @brief Error types for UpdateDisplay application
*/
typedef enum{
    /** No error */
	UPDATEDISPLAY_ERR_NONE,
    /** Error putting command in fifo */
	UPDATEDISPLAY_ERR_FIFO_PUT,     
    /** Error popping command from fifo */
	UPDATEDISPLAY_ERR_FIFO_POP,     
    /** Error parsing component/val in SetComponent */
	UPDATEDISPLAY_ERR_PARSE_COMP,   
    /** Driver call returned an error */
	UPDATEDISPLAY_ERR_DRIVER        
} UpdateDisplayError_t;

/**
 * @enum TriState_t
 * @brief Three states for display elements
*/
typedef enum{
    /** State 0 */
	STATE_0	= 0,
    /** State 1 */ 
	STATE_1	= 1,
    /** State 2 */ 
	STATE_2	= 2
} TriState_t;

// For cruise control and regen
/**
 * @def DISP_DISABLED
 * @brief Disabled state for cruise control and regen
*/
#define DISP_DISABLED STATE_0

/**
 * @def DISP_ENABLED
 * @brief Enabled state for cruise control and regen (able to be used)
*/
#define DISP_ENABLED STATE_1

/**
 * @def DISP_ACTIVE
 * @brief Active state for cruise control and regen (currently being used)
*/
#define DISP_ACTIVE STATE_2

// For gear changes
/**
 * @def DISP_NEUTRAL
 * @brief Neutral state for gear changes
*/
#define DISP_NEUTRAL STATE_0

/**
 * @def DISP_FORWARD
 * @brief Forward state for gear changes
*/
#define DISP_FORWARD STATE_1

/**
 * @def DISP_REVERSE
 * @brief Reverse state for gear changes
*/
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
*/
void UpdateDisplay_ClearQueue(void);

#endif


