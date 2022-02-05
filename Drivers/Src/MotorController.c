#include "MotorController.h"
#include "CarState.h"

#define MOTOR_DRIVE 0x221
#define MOTOR_STATUS 0x241
#define MOTOR_VELOCITY 0x243
#define MAX_CAN_LEN 8

#define MASK_MOTOR_TEMP_ERR 1<<6 //check if motor temperature is an issue on bit 6
#define MASK_SS_ERR 1<<19 //check for slip or hall sequence position error on 19 bit
#define MASK_CC_ERR 1<<2 //checks velocity on 2 bit
#define MASK_OVER_SPEED_ERR 1<<24 //check if motor overshot max RPM on 24 bit

/**
 * @brief   Initializes the motor controller
 * @param   None
 * @return  None
 */ 
void MotorController_Init(){
    BSP_CAN_Init(CAN_3);
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
    
    BSP_CAN_Write(CAN_3, MOTOR_DRIVE, data, MAX_CAN_LEN);
}

/**
 * @brief   Reads most recent command from CAN2 bus
 * @param   message the buffer in which the info for the CAN message will be stored
 * @return  SUCCESS if a message is read
 */ 
ErrorStatus MotorController_Read(CANbuff *message, car_state_t *car_state){

    uint32_t id;
    uint8_t data[8] = {0};
    uint32_t length = BSP_CAN_Read(CAN_3, &id, data);
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

        union {
            uint32_t n;
            float f;
        } convert;
        
        switch (id) {
            // If we're reading the output from the Motor Status command (0x241) then 
            // Check the status bits we care about and set flags accordingly
            case MOTOR_STATUS: {
                if(MASK_MOTOR_TEMP_ERR & firstSum)
                {
                    assertTritiumError(car_state, T_TEMP_ERR);
                }

                if(MASK_SS_ERR & firstSum)
                {
                    assertTritiumError(car_state, T_SLIP_SPEED_ERR);
                }

                if(MASK_CC_ERR & firstSum)
                {
                    assertTritiumError(car_state, T_CC_VEL_ERR);
                }

                if(MASK_OVER_SPEED_ERR & firstSum)
                {
                    assertTritiumError(car_state, T_OVER_SPEED_ERR);
                }
                break;
            }
            case MOTOR_DRIVE: {
                convert.n = secondSum;
                car_state->CurrentVelocity = convert.f;
                break;
            }
            default: break;
        }

        return SUCCESS;
    }
    return ERROR;
}
