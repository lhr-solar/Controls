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
    int acceleratorPercentage;
    int brakePercentage;
    int cruiseEnabled;
    int cruiseSet;
    int regenEnabled;
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
        uint32_t input1 = 0x5053;
        uint32_t input2 = 0x4056;
        MotorController_Drive(input1,input2);
        
        CANbuff tester = {0, 0, 0};
        uint32_t id = 0x243;
        uint8_t data[8] = {0x00, 0x55, 0x55, 0x22,     0x11, 0x11, 0x11, 0x11};
        BSP_CAN_Write(CAN_2, id, data, 8);
        bool check = MotorController_Read(&tester);
        printf("BusID: %x\tMsg: %x, %x\tSuccess: %d\n",tester.id, tester.firstNum, tester.secondNum, check); 


        system("clear");

    }
}
