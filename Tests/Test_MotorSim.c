#include "os.h"
#include "CANbus.h"
#include <stdio.h>

//global variables
static int velocity; 
static CANDATA_t oldCD = {CARDATA, 0, 0};
//lookup table goes here
static int ctr = 0;
static 



int main(void){
    while(1){
        CANDATA_t newCD;
        ErrorStatus error = CANbus_Read(&newCD, CAN_NON_BLOCKING, MOTORCAN); //returns data value into newCD

        if(error == ERROR){
            newCD = oldCD;
        }


        ++ctr;
        if(ctr == 3){ //send message to read canbus every 300 ms
            ctr = 0;
            CANbus_Send(newCD, CAN_NON_BLOCKING, MOTORCAN);
        }

    }
}


