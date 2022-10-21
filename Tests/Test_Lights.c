#include "common.h"
#include "config.h"
#include "Lights.h"
#include <unistd.h>

int main() {
    Lights_Init();
    printf("M_CNCTR\tRSVD_LED\tCTRL_FAULT\tRIGHT_BLINK\tHeadlightPWR\tA_CNCTR\tBPS_FAULT\tLEFT_BLINK\tBPS_PWR\tBrakeLight\n");
    while (1) {
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t%d\t\t%d\t\t%d\t%d\r", 
                Lights_Read(M_CNCTR),
                Lights_Read(RSVD_LED),
                Lights_Read(CTRL_FAULT),
                Lights_Read(RIGHT_BLINK),
                Lights_Read(Headlight_ON),
                Lights_Read(A_CNCTR),
                Lights_Read(BPS_FAULT),
                Lights_Read(LEFT_BLINK),
                Lights_Read(BPS_PWR),
                Lights_Read(BrakeLight));
        fflush(stdout);  
        light_t light = rand() % 10;
        bool state = Lights_Read(light);
        Lights_Set(light, !state);
        usleep(10000);
    }
    printf("\n");
}
