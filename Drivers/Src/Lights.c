/* Copyright (c) 2020 UT Longhorn Racing Solar */
/*  
    Allows for reading from and writing to light states. Reading and writing 
    brake lights has been separated from implementation of other lights because
    brake lights have no need for SPI, as they work solely externally and through GPIO.
    
    Headlights and blinkers need to use SPI and GPIO, given that they have an indicator
    internally as well as an external component.
*/


#include "Lights.h"
#include "common.h"
#include <unistd.h>
#include "BSP_SPI.h"
#include "stm32f4xx.h"


/**
* @brief   Initialize Lights Module
* @return  void
*/ 
void Lights_Init(void) {
    BSP_GPIO_Init(LIGHTS_PORT, 0x3C0, 1); // Pins 6,7,8,9 from Port C are out (0b1111000000)
    BSP_SPI_Init();

    uint8_t txReadBuf[2] = {SPI_OPCODE_R, 0x01}; //0x01 is IODIRB in Bank 0 Mode
    uint8_t rxBuf[1] = {0};
    
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txReadBuf, 2);
    BSP_SPI_Read(rxBuf,1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    uint8_t txWriteBuf[3] = {
        SPI_OPCODE_W, //write to IODIRB
        0x01, 
        rxBuf[0]&0x40 //clear IODIRB to set as outputs except for hazard lights switch pin (bit 6, 0x40)
    };
    
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txWriteBuf, 3);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}

/**
* @brief   Read the state of the lights
* @param   light Which Light to read
* @return  returns state enum which indicates ON/OFF
*/ 
State Lights_Read(light_t light) {
    // Get internal lights from SPI
    uint8_t txBuf[2] = {SPI_OPCODE_R, SPI_GPIOB};
    uint8_t rxBuf[1] = {0};
    
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txBuf, 2);
    BSP_SPI_Read(rxBuf, 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    
    uint8_t currData = rxBuf[0];
    // Different depending on the light
    if (light == BrakeLight) {
        // Only one that is not internal
        return (BSP_GPIO_Read(LIGHTS_PORT) & (0x01 << BRAKELIGHT_PIN)) != OFF;
    } else {
        return (currData & (0x01 << light)) != OFF;
    }
}

/**
 * @brief   Set light to given state
 * @param   light which light to set
 * @param   state what state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, State state) {
    // Get internal lights from SPI
    uint8_t txReadBuf[2] = {SPI_OPCODE_R, SPI_GPIOB};
    uint8_t txWriteBuf[3] = {SPI_OPCODE_W, SPI_GPIOB, 0x00};
    uint8_t rxBuf[1] = {0};
    
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txReadBuf, 2);
    BSP_SPI_Read(rxBuf, 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    
    uint8_t portc = BSP_GPIO_Read(LIGHTS_PORT);
    if (light == BrakeLight) {
        portc &= ~(0x01 << BRAKELIGHT_PIN); // Clear bit corresponding to pin
        portc |= (state << BRAKELIGHT_PIN); // Set value to inputted state
    } else {
        uint16_t currData = rxBuf[0];
        uint16_t newData = currData & ~(0x01 << light);
        newData |= state << light;
        
        txWriteBuf[2] = newData;

        // Lights that are external and internal
        switch (light) {
            case LEFT_BLINK:
                portc &= ~(0x01 << LEFT_BLINK_PIN); // Clear bit corresponding to pin
                portc |= (state << LEFT_BLINK_PIN); // Set value to inputted state
                break;
            case RIGHT_BLINK:
                portc &= ~(0x01 << RIGHT_BLINK_PIN);    // Clear bit corresponding to pin
                portc |= (state << RIGHT_BLINK_PIN);    // Set value to inputted state
                break;
            case Headlight_ON:
                portc &= ~(0x01 << HEADLIGHT_PIN);  // Clear bit corresponding to pin
                portc |= (state << HEADLIGHT_PIN);  // Set value to inputted state
                break;
            default:
                break;
        }
    }

    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txWriteBuf, 3);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    
    BSP_GPIO_Write(LIGHTS_PORT, portc);
}