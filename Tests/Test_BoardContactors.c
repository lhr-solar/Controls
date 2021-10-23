/*
 * Test for board contactors
 */ 

#include "common.h"
#include "config.h"
#include "Contactors.h"


int main(){
    Contactors_Init(MOTOR);
    Contactors_Init(ARRAY);
    while(1){
        Contactors_Set(MOTOR, OFF);
        Contactors_Set(ARRAY, OFF);
        for(int i = 0; i< 999999; i++){} //~0.5 seconds
        Contactors_Set(MOTOR, ON);
        Contactors_Set(ARRAY, ON);
        for(int i = 0; i< 999999; i++){} //~0.5 seconds
    }
}
