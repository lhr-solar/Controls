/**
 * Source file for the library to interact
 * with the switches on the steering wheel
 * 
 * Switch pins are configured as follows:
 * TODO
 */

#include "BSP_Switches.h"

#define FILE_NAME DATA_PATH(SWITCHES_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */ 
void BSP_Switches_Init(void) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(SWITCHES_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads the CSV file and isolates
 *          the specified switch's state
 * @param   sw the switch to read
 * @return  State of the switch (ON/OFF)
 */ 
State BSP_Switches_Read(switch_t sw) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("Switches not available\n\r");
        return 2;
    }
    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Get raw CSV string
    char csv[16];
    fgets(csv, 16, fp);

    // Convert to integer
    uint16_t switchStates = atoi(csv);
    
    // Unlock
    flock(fno, LOCK_UN);
    fclose(fp);
    
    // Check state
    if (switchStates & (1 << sw)) {
        return ON;
    } else {
        return OFF;
    }
}
