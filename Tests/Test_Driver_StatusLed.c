#include "StatusLeds.h"

/*
 * This test should successively toggle all Status LED Pins
 */

int main(){
    Status_Leds_Init();
    while(1){
        for(int i = 0; i < NUM_STATUS_LED; i++){
            Status_Leds_Toggle(i);
            volatile uint32_t waitTime = 0;
            while(waitTime <= 999999){
                waitTime++;
            }
        }
    }
}