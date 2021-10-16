#include "MotorController.h"
#include "CarState.h"
#define MOTOR_DRIVE 0x221
#define MAX_CAN_LEN 8

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void MotorController_Init(){
    BSP_CAN_Init(CAN_2);
}

/**
 * @brief   Sends MOTOR DRIVE command on CAN2
 * @param   newVelocity desired motor velocity setpoint in m/s
 * @param   motorCurrent desired motor current setpoint as a percentage of max current setting
 * @return  None
 */ 
void MotorController_Drive(float newVelocity, float motorCurrent){

    uint32_t nv = *((uint32_t *)((void *) &newVelocity));
    uint32_t mc = *((uint32_t *)((void *) &motorCurrent));

    uint8_t data[8] = {0};
    int index = 0;
    while(index < MAX_CAN_LEN/2){
        data[index] = (mc >> (8 * (MAX_CAN_LEN/2-index-1))) & 0xFF; //split inputs into bytes
        index++;
        
    }
    int i = 0;
    while(index < MAX_CAN_LEN){
        data[index] = (nv >> (8 * (MAX_CAN_LEN/2-i-1))) & 0xFF;
        index++;
        i++;
    }
    
    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, MAX_CAN_LEN);
}

/**
 * @brief   Reads most recent command from CAN2 bus
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  SUCCESS if a message is read
 */ 
ErrorStatus MotorController_Read(CANbuff *message, car_state_t *car){
    car_state_t *car_state = (car_state_t *) car;
    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t length = BSP_CAN_Read(CAN_2, &id, data);
    uint32_t firstSum = 0;
    uint32_t secondSum = 0;

    const uint32_t maskMotorTemp=1<<6; //check if motor temperature is an issue on bit 6
    const uint32_t maskSSErr=1<<19; //check for slip or hall sequence position error on 19 bit
    const uint32_t maskCCErr=1<<2; //checks velocity on 2 bit
    const uint32_t maskOverSpeed=1<<24; //check if motor overshot max RPM on 24 bit

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

        //read first number (bits 0-31) from CAN message to check motor error issue flags
        uint32_t motorTempError=maskMotorTemp & firstSum;
        if(motorTempError==1<<6)
        {
             car.MotorErrorCode.motorTempErr = ON;
        }

        uint32_t SSErr=maskSSErr & firstSum;
        if(SSErr==1<<19)
        {
             car.MotorErrorCode.slipSpeedErr = ON;
        }

        uint32_t CCErr=maskCCErr & firstSum;
        if(CCErr==1<<2)
        {
             car.MotorErrorCode.CCVelocityErr = ON;
        }

        uint32_t motorOverSpeedErr=maskOverSpeed & firstSum;
        if(motorOverSpeedErr==1<<24)
        {
             car.MotorErrorCode.overSpeedErr = ON;
        }

        return SUCCESS;
    }
    return ERROR;
}
