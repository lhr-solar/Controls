/**
 * Test file for prechagre board
 * 
 * 
 */ 

#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    char board[6];
    char status[4];
    char* motor = "MOTOR";
    char* array = "ARRAY";
    char* on = "ON";
    char* off = "OFF";
    
    while(1){
        printf("Enter the board you wish to set\n");
        gets(board);
        printf("Enter the status you wish to set\n");
        gets(status);
        if(!strcmp(board, motor)){
            if(!strcmp(status,on)){
                printf("Writing to csv...\n");
                BSP_Precharge_Write(MOTOR_PRECHARGE, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_Write(MOTOR_PRECHARGE, OFF);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else if(!strcmp(board,array)){
            if(!strcmp(status,on)){
                BSP_Precharge_Write(ARRAY_PRECHARGE, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_Write(ARRAY_PRECHARGE, OFF);
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