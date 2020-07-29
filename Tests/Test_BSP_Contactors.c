#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    BSP_Contactors_Init();
    BSP_Contactor_Set(1,0);
    int16_t states[5];
    read_Contactor_States(states);
    printf("%d%d", states[0], states[1]);
    exit(0);
}