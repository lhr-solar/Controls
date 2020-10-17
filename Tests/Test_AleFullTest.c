#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Display.h"
#include "MotorController.h"
#include <bsp.h>

int main() {
    Pedals_Init();
    Display_Init();
    MotorController_Init();
    BSP_Switches_Init();
    display_data_t packet;
    uint32_t acceleratorPercentage;
    uint32_t brakePercentage;
    uint8_t cruiseEnabled;
    uint8_t cruiseSet;
    uint8_t regenEnabled;
    uint32_t driveVelocity;
    uint32_t driveCurrent;
    uint32_t cruiseVelocity;
    uint32_t cruiseCurrent;
    // bool cruiseLock = false;
    while(1) {
        
        // Pedal Test
        acceleratorPercentage = Pedals_Read(ACCELERATOR);
        brakePercentage = Pedals_Read(BRAKE);
        printf("Accelerator: %5.1d%%,  Brake: %5.1d%%\n", 
            acceleratorPercentage,brakePercentage);
        // Display Test
        // Speed
        if(((acceleratorPercentage * 60)-(brakePercentage * 60)) > 0){
            packet.speed = (((acceleratorPercentage * 60)-(brakePercentage * 60))/100);
        }
        else{
            packet.speed = 0;
        }

        // Cruise / Regen buttons
        packet.cruiseEnabled = BSP_Switches_Read(6);
        packet.cruiseSet = BSP_Switches_Read(5);
        packet.regenEnabled = BSP_Switches_Read(7);
        cruiseEnabled = BSP_Switches_Read(6);
        cruiseSet = BSP_Switches_Read(5);
        regenEnabled = BSP_Switches_Read(7);
        printf("CruiseEn: %d,  CruiseSet: %d,  regenEnabled: %d\n", cruiseEnabled, cruiseSet, regenEnabled);

        Display_SetData(&packet);

        // Motor Controller
        driveVelocity = 0x1000;   //max RPM's desired
        driveCurrent = acceleratorPercentage;   //percentage of current to be used
        cruiseVelocity = 400;   //constant to show "cruise" enabled
        cruiseCurrent = 100;    //max current needed to cruise
        if(cruiseSet==1){
            MotorController_Drive(cruiseVelocity, cruiseCurrent);
        }
        else{
            MotorController_Drive(driveVelocity, driveCurrent);
        }
        // MotorController_Drive(0x1000, 0x1111);
        
        // Motor portion of UI is no longer being updated at all?

        
        // CANbuff tester = {0, 0, 0};
        // uint32_t id = 0x243;
        // uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,     0x11, 0x11, 0x11, 0x11};
        // BSP_CAN_Write(CAN_2, id, data, 8);
        //torque has non-achievable velocity (go with current, not velocity)
        //cruise control uses what current's available to achieve desired velocity (and maintain it)
        // printf("BusID: %x\tMsg: %x, %x\tSuccess: %d\n",tester.id, tester.firstNum, tester.secondNum, check); 


        system("clear");

    }
}
