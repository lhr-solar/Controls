/**
 * @file UpdateDisplay.h
 * @brief Maintains a queue of command structures, allowing other tasks to
 * submit commands and sending out commands to the display as time allows.
 * 
 * Once the UpdateDisplay task has been started, call any of the exposed UpdateDisplay 
 * functions to send a command to the display. Call UpdateDisplay_ClearQueue() to clear the queue.
 * The macros defined in Display.h can be used to set the values of some of the components with more
 * explicit naming.
 * 
 */

#ifndef UPDATE_DISPLAY_H
#define UPDATE_DISPLAY_H

#include "Display.h"
#include "common.h"

/**
 * @brief Error types for UpdateDisplay application
*/
typedef enum{
    /** No error */
	kUpdateDisplayErrNone,
    /** Error putting command in fifo */
	kUpdateDisplayErrFifoPut,     
    /** Error popping command from fifo */
	kUpdateDisplayErrFifoPop,     
    /** Error parsing component/val in SetComponent */
	kUpdateDisplayErrParseComp,   
    /** Driver call returned an error */
	kUpdateDisplayErrDriver        
} UpdateDisplayError;

/**
 * @brief Three states for display elements
*/
typedef enum { kState0 = 0, kState1 = 1, kState2 = 2 } TriState;

// For cruise control and regen
/**
 * @brief Disabled state for cruise control and regen
*/
#define DISP_DISABLED kState0

/**
 * @brief Enabled state for cruise control and regen (able to be used)
*/
#define DISP_ENABLED kState1 

/**
 * @brief Active state for cruise control and regen (currently being used)
*/
#define DISP_ACTIVE kState2

// For gear changes
/**
 * @brief Neutral state for gear changes
*/
#define DISP_NEUTRAL kState0

/**
 * @brief Forward state for gear changes
*/
#define DISP_FORWARD kState1

/**
 * @brief Reverse state for gear changes
*/
#define DISP_REVERSE kState2

/**
 * @brief Initializes UpdateDisplay application
 * @returns UpdateDisplayError_t
 */

UpdateDisplayError UpdateDisplayInit();

/**
 * @brief Selects visible page on the display
 * @param page which page to select
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetPage(Page page);

/**
 * @brief Sets the state of charge value on the display
 * @param percent charge as a percent (0 digits of precision)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetSoc(uint8_t percent);

/**
 * @brief Sets the supplemental battery pack voltage value on the display
 * @param mv supplemental battery pack voltage in millivolts
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetSbpv(uint32_t mv);

/**
 * @brief Sets the velocity of the vehicle on the display
 * @param mphTenths velocity of the vehicle in tenths of mph (1 digit of
 * precision)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetVelocity(uint32_t mph_tenths);

/**
 * @brief Sets the accelerator slider value on the display
 * @param percent pressure on accelerator in percent
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetAccel(uint8_t percent);

/**
 * @brief Sets the array indicator state on the display
 * @param state array contactor on (true) or off (false)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetArray(bool state);

/**
 * @brief Sets the motor contactor indicator state on the display
 * @param state motor contactor on (true) or off (false)
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetMotor(bool state);

/**
 * @brief Sets the gear selection state on the display
 * @param gear DISABLED=N, ENABLED=F, ACTIVE=R
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetGear(TriState gear);

/**
 * @brief Sets the regenerative braking indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetRegenState(TriState state);

/**
 * @brief Sets the cruise control indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns UpdateDisplayError_t
 */
UpdateDisplayError UpdateDisplaySetCruiseState(TriState state);

/**
 * @brief Clears the display message queue and sets the message counter semaphore value to 0
 */
void UpdateDisplayClearQueue(void);

#endif


