#include "common.h"
#include "config.h"
#include "MotorController.h"

int main(){
    
    MotorController_Init();
    uint32_t input1 = 0x5053;
    uint32_t input2 = 0x4056;
    MotorController_Drive(input1,input2);

    CANbuff *tester;
    uint32_t expectedID = 0x243;
    uint32_t id = 0x243;
    uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,     0x11, 0x11, 0x11, 0x11};
    BSP_CAN_Write(CAN_2, id, data, 2);
    bool check = MotorController_Read(tester, expectedID);
    printf("The expected ID is :%x \n\rThe actual ID on the bus was: %x\n\rDid they match? : %d\n\r%x, %x", expectedID, tester->id, check,tester->firstNum, tester->secondNum); 
    exit(0);
    
}