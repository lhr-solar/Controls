#include "MotorController.h"


#define MOTOR_DRIVE 0x221

#define SUCCESS     1
#define FAILURE     0

#define MAX_CAN_LEN 8

void MotorController_Init(){
    BSP_CAN_Init(CAN_2);
}


void MotorController_Drive(uint32_t newVelocity, uint32_t motorCurrent){
    uint8_t data[8] = {0};
    int index = 0;
    while(index < MAX_CAN_LEN/2){
        data[index] = (newVelocity >> (8 * (MAX_CAN_LEN/2-index-1))) & 0xFF;    //split inputs into bytes
        index++;
        
    }
    int i = 0;
    while(index < MAX_CAN_LEN){
        data[index] = (motorCurrent >> (8 * (MAX_CAN_LEN/2-i-1))) & 0xFF;
        index++;
        i++;
    }
    
    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, MAX_CAN_LEN);
}

error_t MotorController_Read(CANbuff *message){
    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t length = BSP_CAN_Read(CAN_2, &id, data);
    uint32_t firstSum = 0;
    uint32_t secondSum = 0;
    if(length>0){
        message->id = id;
        //get first number (bits 0-31)
        for(int j = 0; j < MAX_CAN_LEN/2; j++){
            firstSum += data[j];
            if(j != MAX_CAN_LEN/2 - 1){
                firstSum = firstSum << 8;
            }
        }
        //get second number (bits 32-63)
        for(int k = MAX_CAN_LEN/2; k < MAX_CAN_LEN; k++){
            secondSum += data[k];
            if(k != MAX_CAN_LEN-1){
                secondSum = secondSum << 8;
            }
        }
        message->firstNum = firstSum;
        message->secondNum = secondSum;
        return SUCCESS;
    }
    return FAILURE;
}
