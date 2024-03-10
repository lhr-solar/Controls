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
 * @defgroup UpdateDisplay
 * @addtogroup UpdateDisplay
 * @{
 */

#ifndef UPDATE_DISPLAY_H
#define UPDATE_DISPLAY_H

#include "Display.h"
#include "common.h"

/**
 * Error types
 */
typedef enum {
    kUpdateDisplayErrNone,
    kUpdateDisplayErrFifoPut,    // Error putting command in fifo
    kUpdateDisplayErrFifoPop,    // Error popping command from fifo
    kUpdateDisplayErrParseComp,  // Error parsing component/val in SetComponent
    kUpdateDisplayErrDriver      // Driver call returned an error
} UpdateDisplayError;

/**
 * For display elements with three states
 */
typedef enum { kState0 = 0, kState1 = 1, kState2 = 2 } TriState;

// For cruise control and regen
#define DISP_DISABLED kState0
#define DISP_ENABLED kState1  // Able to be used
#define DISP_ACTIVE kState2   // Actively being used right now

// For gear changes
#define DISP_NEUTRAL kState0
#define DISP_FORWARD kState1
#define DISP_REVERSE kState2

/**
 * Task Prototype
 */
void TaskUpdateDisplay(void* p_arg);

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
 * @brief Clears the display message queue and sets the message counter
 * semaphore value to 0
 * @param none
 * @returns none
 */
void UpdateDisplayClearQueue(void);

#endif
/* @} */
