/**
 * Test file for precharge board
 * 
 * 
 */ 

#include "common.h"
#include "config.h"
#include "Precharge.h"

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
                printf("Writing to board...\n");
                Precharge_Write(MOTOR_PRECHARGE, true);
            }
            else if(!strcmp(status,off)){
                Precharge_Write(MOTOR_PRECHARGE, false);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else if(!strcmp(board,array)){
            if(!strcmp(status,on)){
                Precharge_Write(ARRAY_PRECHARGE, true);
            }
            else if(!strcmp(status,off)){
                Precharge_Write(ARRAY_PRECHARGE, false);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else{
            printf("unrecognized board\n");
        }
        printf("State of board:\n");
        printf("Array: %d  Motor: %d\n", Precharge_Read(ARRAY_PRECHARGE), Precharge_Read(MOTOR_PRECHARGE));
        usleep(1000);
    }
}