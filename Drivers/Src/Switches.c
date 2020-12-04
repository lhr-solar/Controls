#include "Switches.h"

//Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
//Readwrite bit = Read: 1, Write: 0
void Switches_Init(void){
    BSP_SPI_Init();
    //1)Read IODIRA: Address x00
    uint8_t initData[3]={SPI_OPCODE_R, SPI_IODIRA, 0x00};
    BSP_SPI_Write(initData,3);
    uint8_t initReadData[1] = {0};
    //do{
        BSP_SPI_Read(initReadData, 1);
    //}while(initReadData[0] == SPI_IODIRA);
    // OR IODIRA to isolate pins 0-6
    initData[2] = initReadData[0]|0x7F;
    //Write result of OR to IODIRA
    initData[0]=0x40;
    BSP_SPI_Write(initData,3);
};

State Switches_Read(switches_t sw){
    uint8_t query[2]={0x41,0x09}; //query GPIOA
    BSP_SPI_Write(query,2);
    uint8_t SwitchReadData[1] = {0};
    //BSP_SPI_Read(SwitchReadData,1);
    //do{
        BSP_SPI_Read(SwitchReadData,1);
    //}while(SwitchReadData[0] == SPI_IODIRA);

     if (SwitchReadData[0] & (1 << sw)) {
        return ON;
    } else {
        return OFF;
    }
};

