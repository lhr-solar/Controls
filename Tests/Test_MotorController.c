#include "common.h"
#include "config.h"
#include "MotorController.h"

int main(){
    
    MotorController_Init();
    uint32_t input1 = 0x5053;
    uint32_t input2 = 0x4056;
    MotorController_Drive(input1,input2);

    CANbuff *tester;
    uint32_t id = 0x243;
    uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,     0x11, 0x11, 0x11, 0x11};
    BSP_CAN_Write(CAN_2, id, data, 8);
    bool check = MotorController_Read(tester);
    printf("The ID on the bus was: %x\n\rMessage: %x, %x\n\rSuccess: %d",tester->id, tester->firstNum, tester->secondNum, check); 
    exit(0);
    
}