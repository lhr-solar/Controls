/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_Contactors.h"

#define FILE_NAME DATA_PATH(CONTACTORS_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and creates
 *          one if not
 * @param   contactor the contactor to initialize
 *          (not used for the simulator)
 * @return  None
 */ 
void BSP_Contactors_Init(contactor_t contactor) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(CONTACTORS_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads contactor states from a file 
 *          and returns the current state of 
 *          a specified contactor
 * @param   contactor specifies the contactor for 
 *          which the user would like to know its state (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
State BSP_Contactors_Get(contactor_t contactor) {
    // Opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    // Lock file
    int fno = fileno(filePointer);
    flock(fno, LOCK_EX);
    uint8_t contactorStates[NUM_CONTACTORS];
    for (uint8_t i = 0; i < NUM_CONTACTORS; i++) {
        // If the file doesn't contain any more values, stop early
        if (fscanf(filePointer, "%hhd,", &contactorStates[i]) <= 0) break;
    }
    // Closing the file
    flock(fno, LOCK_UN);
    fclose(filePointer);
    return (State) contactorStates[contactor];
} 

/**
 * @brief   Sets the state of a specified contactor
 *          by updating csv file
 * @param   contactor specifies the contactor for 
 *          which the user would like to set its state (MOTOR/ARRAY)
 * @param   state specifies the state that 
 *          the user would like to set (ON/OFF)
 * @return  The contactor's state (ON/OFF)
 */ 
void BSP_Contactors_Set(contactor_t contactor, State state) {
    // Get current values
    State currentStates[NUM_CONTACTORS];
    for (uint8_t i = 0; i < NUM_CONTACTORS; i++) {
        currentStates[i] = BSP_Contactors_Get(i);
    }
    FILE *filePointer = fopen(FILE_NAME, "w");
    // Lock file
    int fno = fileno(filePointer);
    flock(fno, LOCK_EX);

    for (uint8_t i = 0; i < NUM_CONTACTORS; i++) {
        if (contactor == i) {
            fprintf(filePointer, "%d,", state);
        } else {
            fprintf(filePointer, "%d,", currentStates[i]);
        }
    }
    // Closing the file
    flock(fno, LOCK_UN);
    fclose(filePointer);
}
