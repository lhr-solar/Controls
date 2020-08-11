#include "common.h"
#include "config.h"
#include "MotorController.h"

int main(){
    
    Motor_Controller_Init();
    uint32_t input1 = 5;
    uint32_t input2 = 4;
    Motor_Controller_Drive(input1,input2);

    uint32_t velocity;
    uint32_t angularVel;
    bool test = Motor_Controller_ReadVelocity(&angularVel, &velocity);
    printf("\n\r%d %d %d", test, angularVel, velocity);
    exit(0);
    
}