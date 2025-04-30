#include "DebugIO.h"

void DebugIO_Init(void) {
    for (int i = 0; i < NUM_PINS; i++) {
        BSP_GPIO_Init(debugGPIOLUT[i].port, debugGPIOLUT[i].pin, OUTPUT, false);
    }
}

void DebugIO_Write(pinIndex_t pin, bool state) {
    if (pin < NUM_PINS) {
        BSP_GPIO_Write_Pin(debugGPIOLUT[pin].port, debugGPIOLUT[pin].pin, state);
    }
}

void DebugIO_Toggle(pinIndex_t pin) {
    if (pin < NUM_PINS) {
        BSP_GPIO_Toggle_Pin(debugGPIOLUT[pin].port, debugGPIOLUT[pin].pin);
    }
}