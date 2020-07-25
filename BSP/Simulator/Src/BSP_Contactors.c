#include "BSP_Contactors.h"

#define FILE_NAME DATA_PATH(CONTACTORS_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
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
 * @brief   Helper function that reads all of the ADC values 
 *          from a file with raw values provided 
 *          by the ADC and stores all values in local 
 *          array of max capacity 10
 * @param   rawADCValues pointer to array that contains the ADC values in the array
 * @return  None
 *  (adapted from read_ADC_Values in BSP_ADC.c)
 */ 
static void read_Contactor_States(int16_t *contactorStates) {
    // Opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    // Lock file
    int fno = fileno(filePointer);
    flock(fno, LOCK_EX);
    for(int i = 0; i < 10; i++){
        // If the file doesn't contain any more values, stop early
        if (fscanf(filePointer,"%hd,", &(contactorStates[i])) <= 0) break;
    }
    // Closing the file
    flock(fno, LOCK_UN);
    fclose(filePointer);
} 

static void BSP_Contactor_Set(int16_t motorContactorState, int16_t arrayContactorState) {
    FILE *filePointer = fopen(FILE_NAME, "w");
    fprintf(filePointer, "%d %d", motorContactorState, arrayContactorState);
    fclose(filePointer);
}
