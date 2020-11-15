#include "Switches.h"

//Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
//Readwrite bit = Read: 1, Write: 0
void Switches_Init(void){
    BSP_SPI_Init();
    //1)Read IODIRA: Address x00
    uint8_t initData[3]={0x41,0x00,0x00};
    BSP_SPI_Write(initData,3);
    uint8_t initReadData;
    BSP_SPI_Read(initReadData,1);
    // OR IODIRA to isolate pins 0-6
    initData[2] = initReadData|0x7F;
    //Write result of OR to IODIRA
    initData[0]=0x40;
    BSP_SPI_Write(initData,3);
};

State Switches_Read(switches_t sw){
    uint8_t query[2]={0x41,0x00};
    BSP_SPI_Write(query,2);
    uint8_t SwitchReadData;
    BSP_SPI_Read(SwitchReadData,1);
     if (SwitchReadData & (1 << sw)) {
        return ON;
    } else {
        return OFF;
    }
};

