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
#include "Contactors.h"

int main() {
    Contactors_Init(MOTOR);
    Contactors_Init(ARRAY);
    char input[6];
    char motor[] = "MOTOR";
    char array[] = "ARRAY";
    char on[] = "ON";
    char off[] = "OFF";
    char state[4];
    State result;
    while(1) {
        printf("Enter the Contactor you wish to set followed by the state you would like to set it to\n");
        scanf("%s %s", &input, &state);
        if (strcmp(input, motor) == 0){
            if (strcmp(state, on) == 0){
                Contactors_Set(MOTOR, ON, true);
                result = Contactors_Get(MOTOR);
            }
            else {
                Contactors_Set(MOTOR, OFF, true);
                result = Contactors_Get(MOTOR);
            }
        }
        if (strcmp(input, array) == 0){
            if (strcmp(state, on) == 0){
                Contactors_Set(ARRAY, ON, true);
                result = Contactors_Get(ARRAY);
            }
            else {
                Contactors_Set(ARRAY, OFF, true);
                result = Contactors_Get(ARRAY);
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