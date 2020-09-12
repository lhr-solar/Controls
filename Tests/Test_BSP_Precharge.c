/**
 * Test file for prechagre board
 * 
 * 
 */ 

#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    char input[6];
    char status[4];
    char* motor = "MOTOR";
    char* array = "ARRAY";
    char* on = "ON";
    char* off = "OFF";
    
    state_t result;
    while(1){
        printf("Enter the board you wish to set followed by the status (MOTOR | ARRAY) (ON | OFF)\n");
        scanf("%s %s", &input, &status);
        if(!strcmp(input, motor)){
            if(!strcmp(status,on)){
                BSP_Precharge_write(Motor, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_write(Motor, OFF);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else if(!strcmp(input,array)){
            if(!strcmp(status,on)){
                BSP_Precharge_write(Array, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_write(Array, OFF);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else{
            printf("unrecognized board\n");
        }
        usleep(1000);
    }
}