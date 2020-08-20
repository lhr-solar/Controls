#include "MotorController.h"


#define MOTOR_DRIVE 0x221

#define SUCCESS     1
#define FAILURE     0



void MotorController_Init(){
    BSP_CAN_Init(CAN_2);
}


void MotorController_Drive(uint32_t newVelocity, uint32_t motorCurrent){
    uint8_t data[8] = {0};
    int index = 0;
    while(index < 4){
        data[index] = (newVelocity >> (8 * (4-index-1))) & 0xFF;    //split inputs into bytes
        index++;
        
    }
    int i = 0;
    while(index < 8){
        data[index] = (motorCurrent >> (8 * (4-i-1))) & 0xFF;
        index++;
        i++;
    }
    
    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, 8);
}

bool MotorController_Read(CANbuff *message, uint32_t expectedID){
    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t length = BSP_CAN_Read(CAN_2, &id, data);
    uint32_t firstSum = 0;
    uint32_t secondSum = 0;
    if((length>0) && (id == expectedID)){
        message->id = expectedID;
        if(length < 5){
            for(int i = 0; i < length; i++){
                firstSum += data[i] & 0xFF;
                if(i != length-1){
                    firstSum = firstSum << 8;
                }
            }
        }else{
            //get first number (bits 0-31)
            for(int j = 0; j < 4; j++){
                firstSum += data[j] & 0xFF;
                if(j != 3){
                    firstSum = firstSum << 8;
                }
                
            }
            //get second number (bits 32-63)
            for(int k = 4; k < length; k++){
                secondSum += data[k] & 0xFF;
                if(k != length-1){
                    secondSum = secondSum << 8;
                }
            }
        }
        message->firstNum = firstSum;
        message->secondNum = secondSum;
        return SUCCESS;
    }
    return FAILURE;
}
