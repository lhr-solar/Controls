/**
 * @file Pedals.c
 * @details **Note:** The brake pedal that is currently in the car isn't quite working, so 
 * it's been switched over to a GPIO pin. The Pedals driver is still used to access 
 * the brake pedal state, but it's now negative logic; a low pedal percentage means 
 * that the brake pedal is pressed down.
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
 * @brief   Initializes the brake and accelerator
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
int8_t PedalsRead(Pedal pedal) {
    if (pedal >= kNumberOfPedals) {
        return 0;
    }
    int16_t millivolts_pedal =
        (int16_t)BspAdcGetMillivoltage((pedal == kAccelerator) ? kCh10 : kCh11);

    int8_t percentage = 0;

    if (millivolts_pedal >= kLowerBound[pedal]) {
        percentage = (int8_t)((int32_t)(millivolts_pedal - kLowerBound[pedal]) *
                              100 / (kUpperBound[pedal] - kLowerBound[pedal]));
    }

    if (percentage > 100) {
        return 100;
    }
    if (percentage < 0) {
        return 0;
    }

    return percentage;
}
