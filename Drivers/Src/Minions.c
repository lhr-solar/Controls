/* Copyright (c) 2020 UT Longhorn Racing Solar */
#include "Minions.h"

static OS_MUTEX CommMutex; //Mutex to lock SPI lines
//Variables for Switches
static uint16_t switchStatesBitmap = 0;

//Variables for Lights
static uint8_t lightStatesBitmap = 0;   // Stores light states (on/off)
static uint8_t lightToggleBitmap = 0;   // Stores light toggle states (left, right)

//Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
//Readwrite bit = Read: 1, Write: 0

/**
 *  @brief Set chip select low, starting an SPI transmission
 *  @param None
 *  @param None
 */
static void ChipSelect(void) {
    BSP_GPIO_Write_Pin(PORTA, SPI_CS, OFF);
}

/**
 *  @brief Set chip select high, ending an SPI transmission
 *  @param None
 *  @param None
 */
static void ChipDeselect(void) {
    BSP_GPIO_Write_Pin(PORTA, SPI_CS, ON);
}

/**
 * @brief   Initializes all switches
 *          from the steering wheel
 * @param   None
 * @return  None
 */ 
static void Switches_Init(void){
    OS_ERR err;
    CPU_TS timestamp;
    //Sets up pins 0-7 on GPIOA as input 
    uint8_t initTxBuf[3]={SPI_OPCODE_R, SPI_IODIRA, 0};
    uint8_t initRxBuf = 0;

    OSMutexPend(
        &CommMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(0,err);

    ChipSelect();
    BSP_SPI_Write(initTxBuf,2);
    BSP_SPI_Read(&initRxBuf, 1);
    ChipDeselect();
    //OR Result of IODIRA read to set all to 1, then write it back to IODIRA
    initTxBuf[2] = initRxBuf|0xFF;
    initTxBuf[0] = SPI_OPCODE_W;
    ChipSelect();
    BSP_SPI_Write(initTxBuf,3);
    ChipDeselect();
    //Sets up pin 6 on GPIOB as input (for Hazard Switch)
    initTxBuf[0]=SPI_OPCODE_R;
    initTxBuf[1] = SPI_IODIRB;
    initTxBuf[2] = 0;
    initRxBuf = 0;
    ChipSelect();
    BSP_SPI_Write(initTxBuf, 2);
    BSP_SPI_Read(&initRxBuf, 1);
    ChipDeselect();
    //OR IODIRB to set pin 6 to input and write it back (For GPIOExpander, In=1 and Out=0)
    initTxBuf[2] = initRxBuf|0x40;
    initTxBuf[0]=SPI_OPCODE_W;
    ChipSelect();
    BSP_SPI_Write(initTxBuf,3);
    ChipDeselect();

    switchStatesBitmap = 0x0000;

    OSMutexPost(
        &CommMutex,
        OS_OPT_POST_NONE,
        &err
    );
    assertOSError(0,err);
};


/**
* @brief   Initialize Lights Module
* @return  void
*/ 
static void Lights_Init(void) {
    OS_ERR err;
    CPU_TS timestamp;
    BSP_GPIO_Init(LIGHTS_PORT, 0x3C0, 1); // Pins 6,7,8,9 from Port C are out (0b1111000000)
    
    // Initialize txBuf and rxBuf
    uint8_t txReadBuf[2] = {SPI_OPCODE_R, SPI_IODIRB}; //0x01 is IODIRB in Bank 0 Mode
    uint8_t rxBuf = 0;

    OSMutexPend(
        &CommMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(OS_BLINK_LIGHTS_LOC,err);
    
    // Reads direction register and stores in rxBuf
    ChipSelect();
    BSP_SPI_Write(txReadBuf, 2);
    BSP_SPI_Read(&rxBuf,1);
    ChipDeselect();

    uint8_t txWriteBuf[3] = {
        SPI_OPCODE_W, //write to IODIRB
        SPI_IODIRB, 
        rxBuf&0x40 //clear IODIRB to set as outputs except for hazard lights switch pin (bit 6, 0x40)
    };
    
    // Set direction register
    ChipSelect();
    BSP_SPI_Write(txWriteBuf, 3);
    ChipDeselect();

    // Initialize toggle bitmap and states bitmap
    lightToggleBitmap = 0x0000;
    lightStatesBitmap = 0x0000;
    
    // Unlock mutex
    OSMutexPost(
        &CommMutex,
        OS_OPT_POST_NONE,
        &err);
    assertOSError(OS_BLINK_LIGHTS_LOC, err);
}

/**
 * @brief   Initializes all Lights and Switches. **Can be called only after Fault Semaphore is initialized.**
 * @param   None
 * @return  None
 */ 
void Minions_Init(void){
    OS_ERR err;
    OSMutexCreate(&CommMutex, "Communications Mutex", &err);
    assertOSError(0,err);
    BSP_SPI_Init();
    Lights_Init();
    Switches_Init();
}

/**
* @brief   Read the state of a specific light from the light bitmap
* @param   light Which Light to read
* @return  returns State enum which indicates ON/OFF
*/ 
State Lights_Read(light_t light) {
    //Return from a stored state bitmap instead of actually querying hardware. 
    return (State) ((lightStatesBitmap>>light)&0x0001);
}

/**
* @brief   Read the lights bitmap
* @return  returns uint16_t with lights bitmap
*/ 
uint16_t Lights_Bitmap_Read() {
    //Return from a stored state array instead of actually querying hardware.
    return lightStatesBitmap;
}

/**
 * @brief   Reads from static variable bitmap holding values of switches
 * @param   sw
 * @return  ON/OFF State
 */ 
State Switches_Read(switches_t sw){
    return (State) ((switchStatesBitmap >> sw) & 0x0001);
}

/**
 * @brief   Queries all switch-related hardware and updates our internal state with the most recent switch values 
 * that Switches_Read depends on.
 * 
 */ 
void Switches_UpdateStates(void){
    OS_ERR err;
    CPU_TS timestamp;
    uint8_t query[2]={SPI_OPCODE_R,SPI_GPIOA}; //query GPIOA
    uint8_t SwitchDataReg1 = 0, SwitchDataReg2 = 0;

    //Read all switches except for ignition and hazard
    OSMutexPend(
        &CommMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err
    );
    assertOSError(0,err);

    ChipSelect();      
    BSP_SPI_Write(query,2);
    BSP_SPI_Read(&SwitchDataReg1,1);
    ChipDeselect();
    
    //Read Hazard Switch
    query[1] = SPI_GPIOB;

    ChipSelect();
    BSP_SPI_Write(query,2);
    BSP_SPI_Read(&SwitchDataReg2,1);
    ChipDeselect();

    OSMutexPost(
        &CommMutex,
        OS_OPT_POST_NONE,
        &err
    );
    assertOSError(0,err);

    //Read Ignition Switch 1
    uint8_t ign1 = BSP_GPIO_Read_Pin(PORTA, GPIO_Pin_1);

    //Read Ignition Switch 2
    uint8_t ign2 = BSP_GPIO_Read_Pin(PORTA, GPIO_Pin_0);
    
    //Store data in bitmap
    switchStatesBitmap = (ign2 << 10) | (ign1 << 9) | (SwitchDataReg2 << 8) | (SwitchDataReg1);
}

/**
 * @brief Toggles a light. Should be used only after Lights_Toggle_Set has been called for this light so that we are accurately tracking the enabled and disabled lights
 * @param light Which light to toggle
*/
void Lights_Toggle(light_t light){
    State lightState = Lights_Read(light);
    if(lightState == OFF){
        Lights_Set(light, ON);
    } else {
        Lights_Set(light, OFF);
    }
}

/**

 * @brief   Set light toggling
 * @param   light Which light to enable toggling for
 * @param   state State to set toggling
 * @return  void
 */
void Lights_Toggle_Set(light_t light, State state) {
    // Mutex not needed here because only BlinkLights uses this bitmap
    if(light == LEFT_BLINK){
        lightToggleBitmap |= 0x02;
    }
    else if(light == RIGHT_BLINK){
        lightToggleBitmap |= 0x01;
    }

}

/**
* @brief   Read the state of a specific toggleable light from the toggle bitmap
* @param   light Which Light to read
* @return  returns State enum which indicates ON/OFF
*/
State Lights_Toggle_Read(light_t light) {
    if(light == LEFT_BLINK){
        return (State) (lightToggleBitmap&0x02 >> 1);
    }
    else if(light == RIGHT_BLINK){
        return (State) (lightToggleBitmap&0x01);
    }
    else{
        return OFF;
    }
}

/**
 * @brief   Read toggle bitmap
 * @return  returns uint8_t bitmap for toggle (LEFT/RIGHT)
 */
uint8_t Lights_Toggle_Bitmap_Read(void) {
    return lightToggleBitmap;

}

/**
 * @brief   Set light to given state
 * @param   light Which light to set
 * @param   state What state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, State state) {
    CPU_TS timestamp;
    OS_ERR err;
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
        OSMutexPend(
            &CommMutex,
            0, 
            OS_OPT_PEND_BLOCKING, 
            &timestamp, 
            &err);    // Lock mutex in order to update our lights bitmap and write to SPI
        assertOSError(OS_BLINK_LIGHTS_LOC, err);
        
        lightStatesBitmap = lightNewStates;   // Update lights bitmap

        // Write to GPIOB on the minion board (SPI) for internal lights
        ChipSelect();
        BSP_SPI_Write(txWriteBuf, 3);
        ChipDeselect();

        OSMutexPost(
            &CommMutex,
            OS_OPT_POST_NONE,
            &err); // Unlock mutex
        assertOSError(OS_BLINK_LIGHTS_LOC, err);
    }
}