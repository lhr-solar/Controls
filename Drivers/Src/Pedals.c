/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.c
 * @brief
 *
 */

#include "BSP_ADC.h"

#include "Pedals.h"
#include "daybreak_pins.h"
// #include "stm32f4xx_gpio.h"

// Constants used to tune the pedals
// Indexed using pedal_t
// Refine in testing
static const int16_t LowerBound[NUMBER_OF_PEDALS] = {
    0,   // Accelerator lower bound
    0,   // Brake lower bound
};

static const int16_t UpperBound[NUMBER_OF_PEDALS] = {
    3200, // Accelerator upper bound
    3200, // Brake upper bound
};

/**
 * @brief   Initializes the brake and accelerator by using the
 *          BSP_ADC_Init function with parameters ACCELERATOR
 *          and BRAKE
 * @param   None
 * @return  None
 */
void Pedals_Init() {
    BSP_ADC_Init();
    BSP_GPIO_Init(BRAKE_SW_PORT, BRAKE_SW, INPUT, true);
}

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using
 *          data from calibration testing, and returns it
 * @param   pedal_t, ACCELERATOR or BRAKE as defined in enum
 * @return  percent amount the pedal has been pressed in percentage
 */
uint8_t Pedals_Read(pedal_t pedal) {
    if (pedal >= NUMBER_OF_PEDALS) return 0;
    int16_t millivoltsPedal = (int16_t)BSP_ADC_Get_Millivoltage(pedal);

    int8_t percentage = 0;

    if (millivoltsPedal >= LowerBound[pedal]) {
        percentage = (int8_t)((int32_t)(millivoltsPedal - LowerBound[pedal]) * 100 /
                              (UpperBound[pedal] - LowerBound[pedal]));
    }
    
    percentage = percentage > 100 ? 100 : percentage;
    percentage = percentage < 0 ? 0 : percentage;
    percentage = 100 - percentage;
    return (uint8_t)percentage;
}
