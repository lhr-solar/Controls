#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Display.h"
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
        printf("Accelerator: %5.1d%%,  Brake: %5.1d%%,", 
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
        printf("  CruiseEn: %d,  CruiseSet: %d,  regenEnabled: %d\r", cruiseEnabled, cruiseSet, regenEnabled);

        Display_SetData(&packet);

        // Motor Controller
        


    }
}
