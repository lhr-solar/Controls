#include "Switches.h"
#include <time.h>




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
    uint8_t initRxBuf[2] = {0}; 
    BSP_SPI_Write(initTxBuf,3);
    do{
        BSP_SPI_Read(initRxBuf, 2);
        printf("INITRXBUFVAL: %d|%d\r",initRxBuf[0],initRxBuf[1]);
    }while(initRxBuf[1] == 0x00);
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
    uint8_t SwitchReadData[4] = {0};
    BSP_SPI_Write(query,3);

    // If we are not trying to get the state of the ignition switches
    if(sw != IGN_1 && sw != IGN_2){
        //for loop is for addressing bug where register address is returned as the data of that register
        for (uint8_t i = 0; i <= 1; i++)
        {
            do{
                BSP_SPI_Read(SwitchReadData,3);
                //printf("DOWHILE-READ: %x|%x|%x|%x\n",SwitchReadData[0],SwitchReadData[1],SwitchReadData[2],SwitchReadData[3]);
            }while(SwitchReadData[0] == SPI_OPCODE_R);
        }

        if (SwitchReadData[2] & (1 << sw)) {
            return ON;
        } else {
            return OFF;
        }
    }
    // If we are trying to get the state of the ignition switches
    else{ 
        int ignStates = BSP_GPIO_Read(PORTA);
        if(sw == IGN_1){
            return ignStates & 0x1;
        }else{
            return (ignStates & 0x2) >> 1;
        }
    }
    
}

