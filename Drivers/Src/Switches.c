#include "Switches.h"
#include "os.h"
#include "Tasks.h"
#include "BSP_GPIO.h"

static OS_MUTEX CommMutex; //Mutex to lock SPI lines
static uint16_t SwitchStates_Bitmap;

//Data Structure of SPI module is Opcode+RW,Register Address, then Data as 3 element byte array
//Readwrite bit = Read: 1, Write: 0

/**
 * @brief   Initializes all switches
 *          from the steering wheel
 * @param   None
 * @return  None
 */ 
void Switches_Init(void){
    OS_ERR err;
    CPU_TS timestamp;
    OSMutexCreate(&CommMutex, "communications Mutex", &err);
    assertOSError(0,err);
    BSP_SPI_Init();
    //Sets up pins 0-7 on GPIOA as input 
    uint8_t initTxBuf[3]={SPI_OPCODE_R, SPI_IODIRA, 0};
    uint8_t initRxBuf = 0;
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, OFF);
    BSP_SPI_Write(initTxBuf,2);
    BSP_SPI_Read(&initRxBuf, 1);
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, ON);
    //OR Result of IODIRA read to set all to 1, then write it back to IODIRA
    initTxBuf[2] = initRxBuf|0xFF;
    initTxBuf[0] = SPI_OPCODE_W;
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, OFF);
    BSP_SPI_Write(initTxBuf,3);
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, ON);

    //Sets up pin 7 on GPIOB as input (for ReverseSwitch)
    initTxBuf[0]=SPI_OPCODE_R;
    initTxBuf[1] = SPI_IODIRB;
    initTxBuf[2] = 0;
    initRxBuf = 0;
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, OFF);
    BSP_SPI_Write(initTxBuf, 2);
    BSP_SPI_Read(&initRxBuf, 1);
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, ON);
    //OR IODIRB to set pin 7 to input and write it back
    initTxBuf[2] = initRxBuf|0x40;
    initTxBuf[0]=SPI_OPCODE_W;
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, OFF);
    BSP_SPI_Write(initTxBuf,3);
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, ON);
};

/**
 * @brief   Reads from static variable bitmap holding values of switches
 * @param   sw
 * @return  ON/OFF State
 */ 
State Switches_Read(switches_t sw){
    return (State) ((SwitchStates_Bitmap >> sw) && 0x0001);
}

/**
 * @brief   Sends SPI messages to read switches values. Also reads from GPIO's for 
 *          ignition switch values
 */ 
void Switches_Update(void){
    OS_ERR err;
    CPU_TS timestamp;
    uint8_t query[2]={SPI_OPCODE_R,SPI_GPIOA}; //query GPIOA
    uint8_t SwitchDataReg1 = 0, SwitchDataReg2 = 0;

    //Read all switches except for ignition and hazard
    BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, OFF);
        OSMutexPend(
            &CommMutex,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err
        );
        assertOSError(0,err);        
        BSP_SPI_Write(query,2);
        BSP_SPI_Read(SwitchDataReg1,1);
        OSMutexPost(
            &CommMutex,
            OS_OPT_POST_NONE,
            &err
        );
        assertOSError(0,err);
        BSP_GPIO_Write_Pin(PORTA, GPIO_Pin_4, ON);

        //Read Hazard Switch
        query[1] = SPI_GPIOB;
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
        OSMutexPend(
            &CommMutex,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err
        );
        assertOSError(0,err);
        BSP_SPI_Write(query,2);
        BSP_SPI_Read(SwitchDataReg2,1);
        OSMutexPost(
            &CommMutex,
            OS_OPT_POST_NONE,
            &err
        );
        assertOSError(0,err);
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

        //Read Ignition Switch 1
        uint8_t ign1 = BSP_GPIO_Read_Pin(PORTA, GPIO_Pin_1);

        //Read Ignition Switch 2
        uint8_t ign2 = BSP_GPIO_Read_Pin(PORTA, GPIO_Pin_0);
        
        //Store data in bitmap
        SwitchStates_Bitmap = (ign2 << 10) | (ign1 << 9) | (SwitchDataReg2 << 8) | (SwitchDataReg1);
}
