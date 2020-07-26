/**
 * Source file for the library to interact
 * with both CAN lines in the car
 */

#include "BSP_CAN.h"

#define FILE_NAME DATA_PATH(CAN_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */ 
void BSP_CAN_Init(void) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(CAN_CSV);
        exit(EXIT_FAILURE);
    }
}
