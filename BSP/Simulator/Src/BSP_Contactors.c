#include "BSP_Contactors.h"
#include <stdio.h>

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
        // Contactors.csv file is created
        FILE* file_ptr = fopen(FILE_NAME, "w");
        fclose(file_ptr);
    }
}

/**
 * @brief   Reads contactor states from a file 
 *          and stores all values in local 
 *          array of max capacity 10
 * @param   contactorStates pointer to array that contains the contactor states
 * @return  None
 *  (adapted from read_ADC_Values in BSP_ADC.c)
 */ 
void read_Contactor_States(int16_t *contactorStates) {
    // Opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    // Lock file
    int fno = fileno(filePointer);
    //flock(fno, LOCK_EX);
    for(int i = 0; i < 10; i++){
        // If the file doesn't contain any more values, stop early
        if (fscanf(filePointer,"%hd,", &(contactorStates[i])) <= 0) break;
    }
    // Closing the file
    flock(fno, LOCK_UN);
    fclose(filePointer);
} 

void BSP_Contactor_Set(int16_t motorContactorState, int16_t arrayContactorState) {
    FILE *filePointer = fopen(FILE_NAME, "w");
    fprintf(filePointer, "%d, %d", motorContactorState, arrayContactorState);
    fclose(filePointer);
}
