#include "MotorController.h"

#define MOTOR_DRIVE 0x221
#define VELOCITY_ID 0x243
#define SUCCESS     1
#define FAILURE     0

void Motor_Controller_Init(){
    BSP_CAN_Init(CAN_2);
}


void Motor_Controller_Drive(uint32_t newVelocity, uint32_t motorCurrent){
    uint8_t data[8];
    int index = 0;
    while(index < 4){
        data[index] = newVelocity % 100;
        printf("\n\r%x", data[index]);
        index++;
        newVelocity /= 100;
    }

    while(index < 8){
        data[index] = motorCurrent % 100;
        index++;
        newVelocity /= 100;
        printf("\n\r%x", data[index]);
    }

    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, 8);
}

bool Motor_Controller_Read(uint32_t *rpmBuff, uint32_t *mpsBuff){
    uint32_t id;
    uint8_t data[8];
    uint32_t length = BSP_CAN_Read(CAN_2, &id, data);
    uint32_t rmpSum = 0;
    uint32_t mpsSum = 0;
    if(length && (id == VELOCITY_ID)){
        if(length < 5){
            for(int i = 0; i < 4; i++){
                rmpSum += data[i];
            }
        }else{
            for(int j = 4; j < 4 + (length-4); j++){
                mpsSum += data[j];
            }
        }
        *rpmBuff = rmpSum;
        *mpsBuff = mpsSum;
        return SUCCESS;
    }

    return FAILURE;
}
