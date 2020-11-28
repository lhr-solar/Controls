/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Lights.h"

/**
* @brief   Initialize Lights Module
* @return  void
*/ 
void Lights_Init(void) {
    BSP_GPIO_Init(PORTC);
    BSP_SPI_Init();
}

/**
* @brief   Read the state of the lights
* @param   light Which Light to read
* @return  returns state enum which indicates ON/OFF
*/ 
State Lights_Read(light_t light) {
    // Get internal lights from SPI
    uint8_t txBuf[3] = {SPI_OPCODE_R, SPI_GPIOB, 0x00};
    uint8_t rxBuf[2] = {0};
    BSP_SPI_Write(txBuf, 3);

    // Busy wait until SPI data changes
    // (Should be changed with RTOS later on)
    do {
        BSP_SPI_Read(rxBuf, 2);
    } while (rxBuf[0] == SPI_GPIOB);
    
    uint8_t currData = rxBuf[1];
    // Different depending on the light
    if (light == BrakeLight) {
        // Only one that is not internal
        return BSP_GPIO_Read(PORTC) & 0x80;
    } else {
        return currData & (0x01 << light);
    }
}

/**
 * @brief   Set light to given state
 * @param   light which light to set
 * @param   state what state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, State state) {

}
