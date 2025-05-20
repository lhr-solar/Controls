#include "DebugIO.h"

// TODO: Would be cool to also add support for extra ADC and UART breakout pins

/**
 * @brief   Inits all debug pins
 * @param   None
 * @return  None
 */ 
void DebugIO_Init(void) {
    for (int i = 0; i < NUM_DEBUG_PINS; i++) {
        BSP_GPIO_Init(debugGPIOLUT[i].port, debugGPIOLUT[i].pin, OUTPUT, false);
    }
}

/**
 * @brief   Writes to a debug pin
 * @param   pin the pin to write to. Format is PORT+PIN (eg PC17)
 * @return  None
 */ 
void DebugIO_Write(pinIndex_t pin, bool state) {
    if (pin < NUM_DEBUG_PINS) {
        BSP_GPIO_Write_Pin(debugGPIOLUT[pin].port, debugGPIOLUT[pin].pin, state);
    }
}

/**
 * @brief   Toggles a debug pin
 * @param   pin the pin to write to. Format is PORT+PIN (eg PC17)
 * @return  None
 */ 
void DebugIO_Toggle(pinIndex_t pin) {
    if (pin < NUM_DEBUG_PINS) {
        BSP_GPIO_Toggle_Pin(debugGPIOLUT[pin].port, debugGPIOLUT[pin].pin);
    }
}