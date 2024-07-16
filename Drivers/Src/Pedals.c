/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.c
 * @brief 
 * 
 */

#include "Pedals.h"
#include "stm32f4xx_gpio.h"

// Constants used to tune the pedals
// Indexed using pedal_t
// Refine in testing
static const int16_t LowerBound[NUMBER_OF_PEDALS] = {
    500, // Accelerator lower bound
    2100, // Brake lower bound
};

static const int16_t UpperBound[NUMBER_OF_PEDALS] = {
    1100, // Accelerator upper bound
    3300, // Brake upper bound
};

/**
 * @brief   Initializes the brake and accelerator by using the 
 *          BSP_ADC_Init function with parameters ACCELERATOR
 *          and BRAKE 
 * @param   None
 * @return  None
 */
void Pedals_Init(){
    BSP_ADC_Init();
    BSP_GPIO_Init(PORTC, GPIO_Pin_15, INPUT, true);
}

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided 
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using 
 *          data from calibration testing, and returns it
 * @param   pedal_t, ACCELERATOR or BRAKE as defined in enum
 * @return  percent amount the pedal has been pressed in percentage
 */
int8_t Pedals_Read(pedal_t pedal){
    if (pedal == BRAKE){
        return (BSP_GPIO_Read_Pin(PORTC, GPIO_Pin_15))?100:0;
    }
    
    if (pedal >= NUMBER_OF_PEDALS) return 0;
    int16_t millivoltsPedal = (int16_t) BSP_ADC_Get_Millivoltage(pedal);

    int8_t percentage = 0;
    
    if (millivoltsPedal >= LowerBound[pedal]) {
        percentage = (int8_t) ( (int32_t) (millivoltsPedal - LowerBound[pedal]) * 100 /
         (UpperBound[pedal] - LowerBound[pedal]));
    }

    if (percentage > 100) return 100;
    if (percentage <   0) return   0;

    return percentage;
}
