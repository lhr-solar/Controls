#include "Idle.h"

void IdleTaskHook(void){
    while(1){}
}


void IdleInit(void)
{   
    Status_Leds_Init();
    OS_AppIdleTaskHookPtr = &IdleTaskHook;
}