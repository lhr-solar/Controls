/**
 * Test file for CAN
 * 
 * 
 */ 

#include "common.h"
#include "Tasks.h"
#include "BSP_CAN.h"

void 

int main(){
    BSP_CAN_Init(CAN_1, NULL, NULL, NULL, 0);
    BSP_CAN_Init(CAN_3, NULL, NULL, NULL, 0);



    while(1){};
}
