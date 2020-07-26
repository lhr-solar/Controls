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

uint8_t BSP_CAN1_Write(uint32_t id, uint8_t data[8], uint8_t len) {
    // Get current values in CSV
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("CAN not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    if (len > 8) {
        flock(fno, LOCK_UN);
        fclose(fp);
        return 0;
    }

    char currentCAN[2][128];
    char csv[128];
    for (uint8_t i = 0; fgets(csv, 128, fp); i++) {
        strcpy(currentCAN[i], csv);
    }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);

    // Open to write
    fp = fopen(FILE_NAME, "w");

    // Lock file
    fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Write ID & Data (CAN1)
    fprintf(fp, "0x%.3x,0x", id);
    for (uint8_t i = 0; i < len; i++) {
        fprintf(fp, "%.2x", data[i]);
    }

    // Write ID & Data (CAN2)
    fprintf(fp, "\n%s", currentCAN[1]);

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
    return 1;
}

uint8_t BSP_CAN1_Read(uint32_t *id, uint8_t *data) {
    return 1;
}

uint8_t BSP_CAN2_Write(uint32_t id, uint8_t data[8], uint8_t len) {
    return 1;
}

uint8_t BSP_CAN2_Read(uint32_t *id, uint8_t *data) {
    return 1;
}
