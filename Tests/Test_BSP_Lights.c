#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    BSP_Lights_Init();
    printf("M_CNCTR\tRSVD_LED\tCTRL_FAULT\tRIGHT_BLINK\tHeadlightPWR\tA_CNCTR\tBPS_FAULT\tLEFT_BLINK\tBPS_PWR\tBrakeLight\n");
    while(1) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r", 
                BSP_Lights_Read(M_CNCTR),
                BSP_Lights_Read(RSVD_LED),
                BSP_Lights_Read(CTRL_FAULT),
                BSP_Lights_Read(RIGHT_BLINK),
                BSP_Lights_Read(HeadlightPWR),
                BSP_Lights_Read(A_CNCTR),
                BSP_Lights_Read(BPS_FAULT),
                BSP_Lights_Read(LEFT_BLINK),
                BSP_Lights_Read(BPS_PWR),
                BSP_Lights_Read(BrakeLight));
        BSP_Lights_Switch(M_CNCTR);
        BSP_Lights_Switch(RSVD_LED);
        BSP_Lights_Switch(CTRL_FAULT);
        BSP_Lights_Switch(RIGHT_BLINK);
        BSP_Lights_Switch(HeadlightPWR);
        BSP_Lights_Switch(A_CNCTR);
        BSP_Lights_Switch(BPS_FAULT);
        BSP_Lights_Switch(LEFT_BLINK);
        BSP_Lights_Switch(BPS_PWR);
        BSP_Lights_Switch(BrakeLight);
    }
    printf("\n");
}