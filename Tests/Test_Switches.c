#include "common.h"
#include "config.h"
#include "Switches.h"

int main() {
    Switches_Init();
    printf("CRUZ_st\tCRUZ_EN\tHZD_SQ\tFR_SW\tHeadlight_SW\tLEFT_SW\tRIGHT_SW\t\n");
    
    while(1){
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\r",
            Switches_Read(CRUZ_st),
            Switches_Read(CRUZ_EN),
            Switches_Read(HZD_SQ),
            Switches_Read(FR_SW),
            Switches_Read(Headlight_SW),
            Switches_Read(LEFT_SW),
            Switches_Read(RIGHT_SW));
        //printf("inside loop\r");
    }
    printf("\n");
}