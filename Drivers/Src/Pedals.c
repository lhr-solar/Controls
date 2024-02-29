/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.c
 * @brief
 *
 */

#include "Pedals.h"

// Constants used to tune the pedals
// Indexed using Pedal
// Refine in testing
static const int16_t kLowerBound[kNumberOfPedals] = {
    400,   // Accelerator lower bound
    2100,  // Brake lower bound
};

static const int16_t kUpperBound[kNumberOfPedals] = {
    900,   // Accelerator upper bound
    3300,  // Brake upper bound
};

/**
 * @brief   Initializes the brake and accelerator by using the
 *          BspAdcInit function with parameters ACCELERATOR
 *          and BRAKE
 * @param   None
 * @return  None
 */
void PedalsInit() { BspAdcInit(); }

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using
 *          data from calibration testing, and returns it
 * @param   Pedal, ACCELERATOR or BRAKE as defined in enum
 * @return  percent amount the pedal has been pressed in percentage
 */
uint8_t PedalsRead(Pedal pedal) {
    if (pedal >= kNumberOfPedals) {
        return 0;
    }

    uint16_t millivolts_pedal =
        BspAdcGetMillivoltage((pedal == kAccelerator) ? kCh10 : kCh11);

    // Handle case if above bound
    if (millivolts_pedal >= kUpperBound[pedal]) {
        return 100;
    }

    uint8_t percentage = 0;

    // Converts from millivolts to percent using pedal bounds
    percentage = (uint8_t)(((millivolts_pedal - kLowerBound[pedal]) * 100) /
                           (kUpperBound[pedal] - kLowerBound[pedal]));

    return percentage;
}
