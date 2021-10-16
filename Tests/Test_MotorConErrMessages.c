/**
 * File for testing Motor Controller, verifying CAN2 functionality and checking if motor error flags work
 * 
 * The user sets "id" to be the desired Motor Controller command ID 
 * and "data[8]" to be the desired message. The test outputs the id 
 * and message that is received by the Motor Controller. "Success" 
 * is 1 if the message recieved is the expected message.
 */ 
#include "common.h"
#include "config.h"
#include "CarState.h"
#include "MotorController.h"


int main(){

    car_state_t car ; //car_state object, don't need a reference
    MotorController_Init();
    float input1 = 2500.0f;
    float input2 = 1.0f;
    MotorController_Drive(input1,input2);
    usleep(1000000);
    CANbuff tester = {0, 0, 0}; //CANbuff object
    uint32_t id = 0x243;
    uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,  0x11, 0x11, 0x11, 0x11}; //change data so it takes the message that sets all motor error flags
    //BSP_CAN_Write(CAN_2, id, data, 8);
    //need to figure out how to show if the motor error messages flag as "on" if triggered
    uint32_t test_data=0x01080044; //32 bit bc motor controller error issues only in first 32 bits of message 

    bool check = MotorController_Read(&tester, car); //returns 1 if fine, but will also set flags 
    //printf("The ID on the bus was: %x\n\rMessage: %x, %x\n\rSuccess: %d",tester.id, tester.firstNum, tester.secondNum, check);

    printf("Error flags for motor temp, slipspeed, CC velocity, overspeed are on: %d\n\r, %d\n\r, %d\n\r, %d\n\r",
    car->MotorErrorCode.motorTempErr,car->MotorErrorCode.slipSpeedErr, car->MotorErrorCode.CCVelocityErr, 
    car->MotorErrorCode.overSpeedErr); 

    test_data=0x00080044
    printf("Error flags for motor temp, slipspeed, CC velocity are on. Overspeed off: %d\n\r, %d\n\r, %d\n\r, %d\n\r",
    car->MotorErrorCode.motorTempErr,car->MotorErrorCode.slipSpeedErr, car->MotorErrorCode.CCVelocityErr,
    car->MotorErrorCode.overSpeedErr); 

    test_data=0x00080040
    printf("Error flags for motor temp, slipspeed are on. CC velocity, Overspeed off: %d\n\r, %d\n\r, %d\n\r, %d\n\r",
    car->MotorErrorCode.motorTempErr,car->MotorErrorCode.slipSpeedErr, car->MotorErrorCode.CCVelocityErr,
    car->MotorErrorCode.overSpeedErr); 

    test_data=0x00000040
    printf("Error flags for motor temp on. Slipspeed, CC velocity, Overspeed off: %d\n\r, %d\n\r, %d\n\r, %d\n\r",
    car->MotorErrorCode.motorTempErr,car->MotorErrorCode.slipSpeedErr, car->MotorErrorCode.CCVelocityErr,
    car->MotorErrorCode.overSpeedErr); 

    test_data=0x00000000
    printf("Error flags for motor temp, slipspeed, CC velocity, Overspeed off: %d\n\r, %d\n\r, %d\n\r, %d\n\r",
    car->MotorErrorCode.motorTempErr,car->MotorErrorCode.slipSpeedErr, car->MotorErrorCode.CCVelocityErr,
    car->MotorErrorCode.overSpeedErr); 

     
    exit(0);
}

