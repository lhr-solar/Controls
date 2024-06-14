/**
 * @file Pedals.h
 * @brief The Pedals driver is a thin wrapper over the ADC interface exposed by
 * the BSP. It calibrates the ADC output in order to get sensible values for the
 * pedals. These calibration parameters are provided in the c file, and can be
 * easily changed if the pedal wiring changes or if further tuning is required.
 *
 */

#ifndef PEDALS_H
#define PEDALS_H

#include "BSP_ADC.h"

/**
 * @brief Used to index the LowerBound and UpperBound arrays
 */
typedef enum { kAccelerator, kBrake, kNumberOfPedals } Pedal;

/**
 * @brief   Initializes the pedals
 */
void PedalsInit(void);

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using
 *          data from calibration testing, and returns it
 * @param   pedal ACCELERATOR or BRAKE
 * @return  percent amount the pedal has been pressed in percentage
 */
int8_t PedalsRead(Pedal pedal);

#endif