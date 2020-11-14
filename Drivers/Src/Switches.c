#include "Switches.h"

void Switches_Init(void){
    BSP_SPI_Init();
    //Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
    //Readwrite bit = Read: 1, Write: 0
    //1)Read IODIRA: Address x00
    uint8_t initData[3]={0x41,0x00,0x00};
    BSP_SPI_Write(initData,3);
    uint8_t initReadData;
    BSP_SPI_Read(initReadData,1);
    // OR IODIRA with 0b01111111
    initData[2] = initReadData|0x7F;
    //Write result of OR to IODIRA
    initData[0]=0x40;
    BSP_SPI_Write(initData,3);
};
State Switches_Read(switches_t sw){
    uint8_t SwitchReadData;
    uint8_t query[2]={0x41,0x00};
    BSP_SPI_Write(query,2);
    BSP_SPI_Read


};

//