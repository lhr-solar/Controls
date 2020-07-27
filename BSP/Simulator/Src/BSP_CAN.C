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

uint8_t BSP_CAN_Write(CAN_t bus, uint32_t id, uint8_t* data, uint8_t len) {
    // Get current values in CSV
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("CAN not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    if (len > 8 || bus > CAN2) {
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

    // Write ID & Data
    if (bus == CAN1) {
        fprintf(fp, "0x%.3x,0x", id);
        for (uint8_t i = 0; i < len; i++) {
            fprintf(fp, "%.2x", data[i]);
        }
        fprintf(fp, ",%d", len);
        fprintf(fp, "\n%s", currentCAN[1]);
    } else {
        fprintf(fp, "%s", currentCAN[0]);
        fprintf(fp, "0x%.3x,0x", id);
        for (uint8_t i = 0; i < len; i++) {
            fprintf(fp, "%.2x", data[i]);
        }
        fprintf(fp, ",%d", len);
    }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
    return len;
}

uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("CAN not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    char currentCAN[2][256];
    char csv[256];
    for (uint8_t i = 0; fgets(csv, 256, fp); i++) {
        strcpy(currentCAN[i], csv);
    }

    // Read values
    uint64_t fullData;
    uint8_t len;
    sscanf(currentCAN[bus], "%x,%lx,%hhd", id, &fullData, &len);

    // Split hex data into bytes
    for (uint8_t i = 0; i < len; i++) {
        data[i] = (fullData >> (8 * (len-i-1))) & 0xFF;
    }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
    return len;
}
