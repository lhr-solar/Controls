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
    //init Display
    Display_Init();
    //init Canbus
    CANbus_Init();


    while(1) {
        //Pedals test
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r",Pedals_Read(ACCELERATOR),Pedals_Read(BRAKE));
        //
    }
}
