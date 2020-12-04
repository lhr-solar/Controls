#include "Switches.h"

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
    //1)Read IODIRA: Address x00
    uint8_t initTxBuf[3]={SPI_OPCODE_R, SPI_IODIRA, 0x00};
    uint8_t initRxBuf[2] = {0,0}; 
    BSP_SPI_Write(initTxBuf,3);
    do{
        BSP_SPI_Read(initRxBuf, 2);
    }while(initRxBuf[0] == SPI_IODIRA);
    
    // OR IODIRA to isolate pins 0-6
    initTxBuf[2] = initRxBuf[1]|0x7F;
    //Write result of OR to IODIRA
    initTxBuf[0]=SPI_OPCODE_W;
    BSP_SPI_Write(initTxBuf,3);
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
    BSP_SPI_Write(query,3);
    //BSP_SPI_Read(SwitchReadData,1);
    do{
        BSP_SPI_Read(SwitchReadData,2);
    }while(SwitchReadData[0] == SPI_GPIOA);

     if (SwitchReadData[1] & (1 << sw)) {
        return ON;
    } else {
        return OFF;
    }
};

