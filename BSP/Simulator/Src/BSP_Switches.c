/**
 * Source file for the library to interact
 * with the switches on the steering wheel
 * 
 * Switch pins are configured as follows:
 * TODO
 */

#include "BSP_Switches.h"

#define FILE_NAME DATA_PATH("Switches.csv")

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
        perror("Switches.csv");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads the CSV file and isolates
 *          the specified switch's state
 * @param   sw the switch to read
 * @return  State of the switch (ON/OFF)
 */ 
state_t BSP_Switches_Read(switch_t sw) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("ADC not available\n\r");
        return 2;
    }
    // Get raw CSV string
    char csv[16];
    fgets(csv, 16, fp);

    // Convert to integer
    uint8_t switchStates = atoi(csv);
    
    // Check state
    if (switchStates & (1 << sw)) {
        return ON;
    } else {
        return OFF;
    }
}
