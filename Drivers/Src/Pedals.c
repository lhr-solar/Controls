/**
 * C file that initializes the accelerator and brake pedals
 * and reads the position of the pedals to see how far the 
 * pedal has been pressed
 */

#include "Pedals.h"

/**
 * @brief   Initializes the brake and accelerator by using the 
 *          BSP_ADC_Init function with parameters ACCELERATOR
 *          and BRAKE 
 * @param   None
 * @return  None
 */
void Pedals_Init(){
    BSP_ADC_Init(ACCELERATOR);
    BSP_ADC_Init(BRAKE);
}

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided 
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed and 
 *          returns it
 * @param   pedal_t, ACCELERATOR or BRAKE as defined in enum
 * @return  distance the pedal has been pressed in percentage
 */
int8_t Pedals_Read(pedal_t pedal){
    int16_t millivolts_Pedal = BSP_ADC_Get_Millivoltage(pedal);
    int8_t percentage = millivolts_Pedal * 100 / ADC_RANGE_MILLIVOLTS;
    return percentage;
}

