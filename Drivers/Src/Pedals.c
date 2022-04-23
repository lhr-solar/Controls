/**
 * C file that initializes the accelerator and brake pedals
 * and reads the position of the pedals to see how far the 
 * pedal has been pressed
 */

#include "Pedals.h"

// Constants used to tune the pedals
// Indexed using pedal_t
// Refine in testing
static const int16_t LowerBound[NUMBER_OF_PEDALS] = {
    400, // Accelerator lower bound
    2000, // Brake lower bound
};

static const int16_t UpperBound[NUMBER_OF_PEDALS] = {
    900, // Accelerator upper bound
    ADC_RANGE_MILLIVOLTS, // Brake upper bound
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
}

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided 
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using 
 *          data from calibration testing, and returns it
 * @param   pedal_t, ACCELERATOR or BRAKE as defined in enum
 * @return  distance the pedal has been pressed in percentage
 */
int8_t Pedals_Read(pedal_t pedal){
    if (pedal >= NUMBER_OF_PEDALS) return 0;
    int16_t millivoltsPedal = (int16_t) BSP_ADC_Get_Millivoltage(pedal);

    int8_t percentage = 0;
    
    if (millivoltsPedal >= LowerBound[pedal]) {
        percentage = (int8_t) ( (int32_t) (millivoltsPedal - LowerBound[pedal]) * 100 /
         (UpperBound[pedal] - LowerBound[pedal]));
    }

    return percentage;
}

