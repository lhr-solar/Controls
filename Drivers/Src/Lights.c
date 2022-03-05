/* Copyright (c) 2020 UT Longhorn Racing Solar */
/*  
    Allows for reading from and writing to light states. Reading and writing 
    brake lights has been separated from implementation of other lights because
    brake lights have no need for SPI, as they work solely externally and through GPIO.
    
    Headlights and blinkers need to use SPI and GPIO, given that they have an indicator
    internally as well as an external component.
*/


#include "Lights.h"
#include "BSP_SPI.h"
#include "stm32f4xx.h"
#include "Tasks.h"

//static State lightStates[10] = {OFF};
static uint8_t lightStatesBitmap;   // Stores light states (on/off)
static uint8_t lightToggleBitmap;   // Stores light toggle states (flashing/not flashing)
static OS_MUTEX lightMutex;
static CPU_TS timestamp;
static OS_ERR err;

/**
* @brief   Initialize Lights Module
* @return  void
*/ 
void Lights_Init(void) {
    OSMutexCreate(&lightMutex, "Light lock", &err); //init mutex
    assertOSError(OS_BLINK_LIGHTS_LOC, err);

    OSMutexPend(&lightMutex,0,OS_OPT_PEND_BLOCKING,&timestamp,&err); //lock light hardware for initialization (TODO: may not be needed, just here for additional safety)
    assertOSError(OS_BLINK_LIGHTS_LOC, err);

    BSP_GPIO_Init(LIGHTS_PORT, 0x3C0, 1); // Pins 6,7,8,9 from Port C are out (0b1111000000)
    BSP_SPI_Init();

    // Initialize txBuf and rxBuf
    uint8_t txReadBuf[2] = {SPI_OPCODE_R, SPI_IODIRB}; //0x01 is IODIRB in Bank 0 Mode
    uint8_t rxBuf = 0;
    
    // Reads direction register and stores in rxBuf
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txReadBuf, 2);
    BSP_SPI_Read(&rxBuf,1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    uint8_t txWriteBuf[3] = {
        SPI_OPCODE_W, //write to IODIRB
        SPI_IODIRB, 
        rxBuf&0x40 //clear IODIRB to set as outputs except for hazard lights switch pin (bit 6, 0x40)
    };
    
    // Set direction register
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(txWriteBuf, 3);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    // Initialize toggle bitmap and states bitmap
    lightToggleBitmap = 0x0000;
    lightStatesBitmap = 0x0000;
    
    // Unlock mutex
    OSMutexPost(&lightMutex,OS_OPT_POST_NONE,&err);
    assertOSError(OS_BLINK_LIGHTS_LOC, err);
}

/**
* @brief   Read the state of a specific light from the light bitmap
* @param   light Which Light to read
* @return  returns State enum which indicates ON/OFF
*/ 
State Lights_Read(light_t light) {
    //Return from a stored state bitmap instead of actually querying hardware.
    return (lightStatesBitmap>>light)&0x01;
}

/**
* @brief   Read the lights bitmap
* @return  returns uint16_t with lights bitmap
*/ 
uint16_t Lights_Bitmap_Read(light_t light) {
    //Return from a stored state array instead of actually querying hardware.
    return lightStatesBitmap;
}

/**

 * @brief   Set light toggling
 * @param   light Which light to enable toggling for
 * @param   state State to set toggling
 * @return  void
 */
void Toggle_Set(light_t light, State state) {
    // Mutex not needed here because only BlinkLights uses this bitmap
    if(state==ON)
        lightToggleBitmap |= (0x01<<light);
    else if(state==OFF)
        lightToggleBitmap &= ~(0x01<<light);
}

/**
 * @brief Toggles a light. Should be used only after Toggle_Enable has been called for this light so that we are accurately tracking the enabled and disabled lights
 * @param light Which light to toggle
*/
void Toggle_Light(light_t light){
    State lightState = Lights_Read(light);
    if(lightState == OFF){
        Lights_Set(light, ON);
    } else {
        Lights_Set(light, OFF);
    }
}

/**
* @brief   Read the state of a specific toggleable light from the toggle bitmap
* @param   light Which Light to read
* @return  returns State enum which indicates ON/OFF
*/
State Toggle_Read(light_t light) {
    return (lightToggleBitmap>>light)&0x01;
}

/**
 * @brief   Read toggle bitmap
 * @return  returns uint16_t bitmap for toggle
 */
uint16_t Toggle_Bitmap_Read() {
    return lightToggleBitmap;

}

/**
 * @brief   Set light to given state
 * @param   light Which light to set
 * @param   state What state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, State state) {
    State lightCurrentState = Lights_Read(light);
    
    if(lightCurrentState != state){     // Check if state has changed
        uint8_t lightNewStates = lightStatesBitmap;
        
        lightNewStates &= ~(0x01 << light); // Clear bit corresponding to pin
        lightNewStates |= (state << light); // Set value to inputted state
        
        // Initialize tx buffer and port c
        uint8_t txWriteBuf[3] = {SPI_OPCODE_W, SPI_GPIOB, 0x00};

        if (light == BrakeLight) {  // Brakelight is only external
            BSP_GPIO_Write_Pin(LIGHTS_PORT, BRAKELIGHT_PIN, ON);
        } else {
            txWriteBuf[2] = lightNewStates; // Write to tx buffer for lights present internally (on minion board)
            
            // Write to port c for lights present externally
            switch (light) {
                case LEFT_BLINK:
                    BSP_GPIO_Write_Pin(LIGHTS_PORT, LEFT_BLINK_PIN, ON);
                    break;
                case RIGHT_BLINK:
                    BSP_GPIO_Write_Pin(LIGHTS_PORT, RIGHT_BLINK_PIN, ON);
                    break;
                case Headlight_ON:
                    BSP_GPIO_Write_Pin(LIGHTS_PORT, HEADLIGHT_PIN, ON);
                    break;
                default:
                    break;
            }
        }
        OSMutexPend(&lightMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);    // Lock mutex in order to update our lights bitmap and write to SPI
        assertOSError(OS_BLINK_LIGHTS_LOC, err);
        
        lightStatesBitmap = lightNewStates;   // Update lights bitmap

        // Write to GPIOB on the minion board (SPI) for internal lights
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
        BSP_SPI_Write(txWriteBuf, 3);
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

        OSMutexPost(&lightMutex,OS_OPT_POST_NONE,&err); // Unlock mutex
        assertOSError(OS_BLINK_LIGHTS_LOC, err);
    }
}


