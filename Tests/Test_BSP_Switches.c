#include "common.h"
#include "config.h"

int main() {
    BSP_Switches_Init();
    printf("LT\tRT\tHDLT\tFWD/REV\tHZD\tCRS_SET\tCRS_EN\tREGEN\n");
    while(1) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r", 
                BSP_Switches_Read(LT),
                BSP_Switches_Read(RT),
                BSP_Switches_Read(HDLT),
                BSP_Switches_Read(FWD_REV),
                BSP_Switches_Read(HZD),
                BSP_Switches_Read(CRS_SET),
                BSP_Switches_Read(CRS_EN),
                BSP_Switches_Read(REGEN));
    }
    printf("\n");
}
