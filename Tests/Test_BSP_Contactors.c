/**
 * Test file for setting contactor states
 * 
 * Run this test in conjunction with the simulator 
 * GUI. The user is prompted to specify the
 * contactor name and the state it should 
 * be set to. The input should be all caps and formatted
 * as such: CONTACTOR_NAME(MOTOR/ARRAY) STATE(ON/OFF)
 * Example inputs: "MOTOR ON", "ARRAY OFF"
 * The corresponding contactor will change state
 * in the GUI.
 */ 

#include "common.h"
#include "config.h"
#include <bsp.h>

int main() {
    BSP_Contactors_Init(MOTOR);
    BSP_Contactors_Init(ARRAY);
    char input[6];
    char motor[] = "MOTOR";
    char array[] = "ARRAY";
    char on[] = "ON";
    char off[] = "OFF";
    char state[4];
    state_t result;
    while(1) {
        printf("Enter the Contactor you wish to set followed by the state you would like to set it to\n");
        scanf("%s %s", &input, &state);
        if (strcmp(input, motor) == 0){
            if (strcmp(state, on) == 0){
                BSP_Contactors_Set(MOTOR, ON);
                result = BSP_Contactors_Get(MOTOR);
            }
            else {
                BSP_Contactors_Set(MOTOR, OFF);
                result = BSP_Contactors_Get(MOTOR);
            }
        }
        if (strcmp(input, array) == 0){
            if (strcmp(state, on) == 0){
                BSP_Contactors_Set(ARRAY, ON);
                result = BSP_Contactors_Get(ARRAY);
            }
            else {
                BSP_Contactors_Set(ARRAY, OFF);
                result = BSP_Contactors_Get(ARRAY);
            }
        }
        usleep(1000);
        if(result == ON) {
            printf("State confirmed: %s\n", on);
        }
        if(result == OFF) {
            printf("State confirmed: %s\n", off);
        }
    } 
}