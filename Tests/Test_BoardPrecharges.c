/**
 * Test for board contactors
 */ 

#include "common.h"
#include "config.h"
#include "Precharge.h"


int main(){
    Precharges_Init(MOTOR_PRECHARGE);
    Precharges_Init(ARRAY_PRECHARGE);
    while(1){
        Precharge_Write(MOTOR_PRECHARGE, OFF);
        Precharge_Write(ARRAY_PRECHARGE, OFF);
        for(int i = 0; i< 999999; i++){} //~0.5 seconds
        Precharge_Write(MOTOR_PRECHARGE, ON);
        Precharge_Write (ARRAY_PRECHARGE, ON);
        for(int i = 0; i< 999999; i++){} //~0.5 seconds
    }
}
