#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    BSP_Contactors_Init();
    BSP_Contactor_Set(1,0);
    int states[5];
    read_Contactor_States(states);
}