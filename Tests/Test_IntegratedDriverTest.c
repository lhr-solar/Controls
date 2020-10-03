#include "common.h"
#include "config.h"

#include "Pedals.h"
#include "Display.h"
#include "MotorController.h"
#include "CANbus.h"






int main() {
    //Init Pedals module
    Pedals_Init();

    //init MotorController 
    MotorController_Init();
    CANbuff MotorControllerTest={0,0,0};

    //init Display
    Display_Init();

    //init Canbus
    CANbus_Init();

    // MotorController Test
    MotorController_Drive(10,10);
    bool ReadCheck = MotorController_Read(&MotorControllerTest); 
    printf("Message Read from Motor Controller Bus?: %d", MotorControllerTest);
    printf("ID of CAN Msg: %x\n\rMessage: %x, %x",MotorControllerTest.id,MotorControllerTest.firstNum,MotorControllerTest.secondNum);

    while(1) {
        //Pedals test
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r",Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
    }
}
