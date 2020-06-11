/**
 * Source file for the library to interact
 * with the switches on the steering wheel
 * 
 * Switch pins are configured as follows:
 * TODO
 */

#include "BSP_Switches.h"

#define FILE DATA_PATH("Switches.csv")

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */ 
void BSP_Switches_Init(void) {
    if (access(FILE, F_OK) != 0) {
        // File doesn't exist if true
        perror("Switches.csv");
        exit(EXIT_FAILURE);
    }
}
