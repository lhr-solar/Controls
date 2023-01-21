#include "os.h"
#include "CANbus.h"
#include <stdio.h>

//global variables
static int velocity; 
static CANDATA_t oldCD;
//lookup table goes here
static int ctr = 0;
static 



int main(void){
    while(1){
        CANDATA_t newCD;
        CANbus_Read(&newCD, CAN_NON_BLOCKING, MOTORCAN); //returns data value into newCD
        



    }
}


