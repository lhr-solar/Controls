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
    float unatttainable_Velocity = 2500.0f;
    float currentSetPoint = 0.05f;
    



    while(1){volatile int x = 0;}
    
    
}