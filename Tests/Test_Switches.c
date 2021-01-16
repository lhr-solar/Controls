#include "common.h"
#include "config.h"
#include "Switches.h"

int main() {
    Switches_Init();
    printf("\n");
    printf("CRUZ_SW\tCRUZ_EN\tHZD_SQ\tFR_SW\tHEADLIGHT_SW\tLEFT_SW\tRIGHT_SW\tREGEN_SW\tIGN_1\tIGN_2\tREV_SW\n");
    while(1){

        printf("%d\t", Switches_Read(CRUZ_SW));
        printf("%d\t", Switches_Read(CRUZ_EN));
        printf("%d\t", Switches_Read(HZD_SQ));
        printf("%d\t", Switches_Read(FR_SW));
        printf("%d\t\t", Switches_Read(HEADLIGHT_SW));
        printf("%d\t", Switches_Read(LEFT_SW));
        printf("%d\t\t", Switches_Read(RIGHT_SW));
        printf("%d\t\t", Switches_Read(REGEN_SW));
        printf("%d\t", Switches_Read(IGN_1));
        printf("%d\t", Switches_Read(IGN_2));
        printf("%d\n", Switches_Read(REV_SW));
    }
}