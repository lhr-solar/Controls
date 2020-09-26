#include "common.h"
#include "config.h"
#include "Pedals.h"
#include "Display.h"
#include <bsp.h>

int main() {
    Pedals_Init();
    Display_Init();
    MotorController_Init();
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
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r", 
            acceleratorPercentage,brakePercentage);
        // Display Test
        //Speed
        if((acceleratorPercentage/100 * 60)-(brakePercentage/100 * 60) > 0){
            packet.speed = (acceleratorPercentage/100 * 60)-(brakePercentage/100 * 60);
        }
        else{
            packet.speed = 0;
        }
        // Cruise / Regen buttons
        packet.cruiseEnabled = BSP_Switches_Read(6);
        packet.cruiseSet = BSP_Switches_Read(5);
        packet.regenEnabled = BSP_Switches_Read(7);
        printf("CruiseEn: %d CruiseSet: %d regenEnabled: %d\r", cruiseEnabled, cruiseSet, regenEnabled);
        // Motor Controller
        


    }
}
