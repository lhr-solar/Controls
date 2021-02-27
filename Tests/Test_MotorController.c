/**
 * File for testing Motor Controller and verifying CAN2 functionality
 * 
 * The user sets "id" to be the desired Motor Controller command ID 
 * and "data[8]" to be the desired message. The test outputs the id 
 * and message that is received by the Motor Controller. "Success" 
 * is 1 if the message recieved is the expected message.
 */ 
#include "common.h"
#include "config.h"
#include "MotorController.h"

int main(){
    
    MotorController_Init();
    float input1 = 2500.0f;
    float input2 = 1.0f;
    MotorController_Drive(input1,input2);
    usleep(1000000);
    CANbuff tester = {0, 0, 0};
    uint32_t id = 0x243;
    uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,     0x11, 0x11, 0x11, 0x11};
    //BSP_CAN_Write(CAN_2, id, data, 8);
    bool check = MotorController_Read(&tester);
    printf("The ID on the bus was: %x\n\rMessage: %x, %x\n\rSuccess: %d",tester.id, tester.firstNum, tester.secondNum, check); 
    exit(0);
    
}