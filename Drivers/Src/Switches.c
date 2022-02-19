#include "Switches.h"
#include "stm32f4xx.h"

/**
 * TODO
 * Change the macros for the registers
 * put the select stuff around the init stuff
 * change the buffers
 * 
 */


//Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
//Readwrite bit = Read: 1, Write: 0

/**
 * @brief   Initializes all switches
 *          from the steering wheel
 * @param   None
 * @return  None
 */ 
void Switches_Init(void){
    BSP_SPI_Init();
    //Sets up pins 0-7 on GPIOA as input 
    uint8_t initTxBuf[3]={SPI_OPCODE_R, SPI_IODIRA, 0};
    uint8_t initRxBuf[1] = {0};
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET); 
    BSP_SPI_Write(initTxBuf,2);
    BSP_SPI_Read(initRxBuf, 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    //OR Result of IODIRA read to set all to 1, then write it back to IODIRA
    initTxBuf[2] = initRxBuf[0]|0xFF;
    initTxBuf[0]=SPI_OPCODE_W;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(initTxBuf,3);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    //Sets up pin 7 on GPIOB as input (for ReverseSwitch)
    initTxBuf[0]=SPI_OPCODE_R;
    initTxBuf[1] = SPI_IODIRB;
    initTxBuf[2] = 0;
    initRxBuf[0]=0;
    initRxBuf[1]=0;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(initTxBuf, 2);
    BSP_SPI_Read(initRxBuf, 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    //OR IODIRB to set pin 7 to input and write it back
    initTxBuf[2] = initRxBuf[1]|0x40;
    initTxBuf[0]=SPI_OPCODE_W;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    BSP_SPI_Write(initTxBuf,3);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

};

/**
 * @brief   Writes to SPI with Write Bit set (no change allowed) to query GPIOA Register (x09), 
 * reads it back using SPI, isolates for input switch using AND mask and returns state 
 * @param   sw
 * @return  ON/OFF State
 */ 
State Switches_Read(switches_t sw){
    uint8_t query[3]={SPI_OPCODE_R,SPI_GPIOA,0x00}; //query GPIOA
    uint8_t SwitchReadData[2] = {0};
    // If we are not trying to get the state of the ignition switches, or the Reverse switch
    switch(sw) {
    case CRUZ_SW:
    case CRUZ_EN:
    case HZD_SQ: 
    case FR_SW: 
    case HEADLIGHT_SW: 
    case LEFT_SW: 
    case RIGHT_SW: 
    case REGEN_SW:
        BSP_SPI_Write(query,3);
        do{
            BSP_SPI_Read(SwitchReadData,2);
        }while(SwitchReadData[0] == SPI_GPIOA);
        if (SwitchReadData[1] & (1 << sw)) {
            return ON;
        } else {
            return OFF;
        }

    case REV_SW:
        query[1] = SPI_GPIOB;
        BSP_SPI_Write(query,3);
        do{
            BSP_SPI_Read(SwitchReadData,2);
        }while(SwitchReadData[0] == SPI_GPIOB);
        if (SwitchReadData[1] & (1 << 7)) {
            return ON;
        } else {
            return OFF;
        }

    case IGN_1: {
        int ignStates = BSP_GPIO_Read(PORTA);
        return (ignStates & 0x2) >> 1;
    }
    case IGN_2: {
        int ignStates = BSP_GPIO_Read(PORTA);
        return (ignStates & 0x1);
    }

    default:
        // Shouldn't happen
        return -1;
    }
    
}

