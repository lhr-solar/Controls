/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.c
 * @brief 
 * 
 * Used for pedal input reading. Each pedal is calibrated with
 * an upper and lower analog bound used to convert ADC readings into 
 * a percentent representing how much each pedal is being pressed.
 *
 */

#include "Pedals.h"
#include "daybreak_pins.h"
#include "stm32f4xx_gpio.h"

// Constants used to tune the pedals
// Indexed using pedal_t
// Refine in testing
static const uint16_t LowerBound[NUMBER_OF_PEDALS] = {
    0,    // Accelerator lower bound
    2220, // Brake lower bound
};

static const uint16_t UpperBound[NUMBER_OF_PEDALS] = {
    3150, // Accelerator upper bound
    3205, // Brake upper bound
};

/**
 * @brief   Initializes the brake and accelerator by using the
 *          BSP_ADC_Init function with parameters ACCELERATOR
 *          and BRAKE
 * @param   None
 * @return  None
 */
void Pedals_Init()
{
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
uint8_t Pedals_Read(pedal_t pedal)
{

    if (pedal >= NUMBER_OF_PEDALS)
        return 0;

    if (pedal == BRAKE)
    {
        return (BSP_GPIO_Read_Pin(PORTC, GPIO_Pin_15)) ? 100 : 0;
    }

    uint16_t millivoltsPedal = BSP_ADC_Get_Millivoltage(pedal);

    int16_t percentage = 0;

    if (millivoltsPedal >= LowerBound[pedal])
    {
        percentage = ((millivoltsPedal - LowerBound[pedal]) * 100 /
                               (UpperBound[pedal] - LowerBound[pedal]));
    }

    if (percentage > 100)
        return 100;
    if (percentage < 0)
        return 0;

    return (uint8_t)percentage;
}
