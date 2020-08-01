#include "BSP_Contactors.h"

#define FILE_NAME DATA_PATH(CONTACTORS_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and creates
 *          one if not
 * @param   None
 * @return  None
 */ 
void BSP_Contactors_Init(void) {
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
state_t BSP_Contactors_Get(ContactorType_t contactor) {
    // Opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    // Lock file
    int fno = fileno(filePointer);
    //flock(fno, LOCK_EX);
    uint16_t contactorStates[2];
    for(int i = 0; i < 2; i++){
        // If the file doesn't contain any more values, stop early
        if (fscanf(filePointer,"%hd,", &contactorStates[i]) <= 0) break;
    }
    // Closing the file
    flock(fno, LOCK_UN);
    fclose(filePointer);
    return contactorStates[contactor];
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
void BSP_Contactors_Set(ContactorType_t contactor, state_t state) {
    state_t state1 = BSP_Contactors_Get(MOTOR);
    state_t state2 = BSP_Contactors_Get(ARRAY);
    FILE *filePointer = fopen(FILE_NAME, "w");
    if(contactor == MOTOR) {
        fprintf(filePointer, "%d, %d", state, state2);
        fclose(filePointer);
    }
    else {
        fprintf(filePointer, "%d, %d", state1, state);
        fclose(filePointer);  
    }
}
