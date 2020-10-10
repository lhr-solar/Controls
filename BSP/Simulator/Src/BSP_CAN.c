/* Copyright (c) 2020 UT Longhorn Racing Solar */

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
 * @param   bus the CAN line to initialize
 *          (not used for simulator)
 * @return  None
 */ 
void BSP_CAN_Init(CAN_t bus) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(CAN_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Writes the id, message, and length 
 *          of the message to the proper 
 *          bus line in the CSV file
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id the hex ID for the message to be sent
 * @param   data pointer to the array containing the message
 * @param   len length of the message in bytes
 * @return  number of bytes transmitted (0 if unsuccessful)
 */
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

    if (len > 8 || bus >= NUM_CAN) {
        flock(fno, LOCK_UN);
        fclose(fp);
        return 0;
    }

    char currentCAN[NUM_CAN][128];
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
    for (uint8_t i = 0; i < NUM_CAN; i++) {
        if (bus == i) {
            fprintf(fp, "0x%.3x,0x", id);
            for (uint8_t i = 0; i < len; i++) {
                fprintf(fp, "%.2x", data[i]);
            }
            fprintf(fp, ",%d\n", len);
        } else {
            fprintf(fp, "%s", currentCAN[i]);
        }
    }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
    return len;
}

/**
 * @brief   Reads the message currently on the 
 *          specified CAN line in the CSV file
 * @param   bus the proper CAN line to write to
 *          defined by the CAN_t enum
 * @param   id pointer to integer to store the 
 *          message ID that was read
 * @param   data pointer to integer array to store
 *          the message in bytes
 * @return  number of bytes read (0 if unsuccessful)
 */
uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("CAN not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    char currentCAN[NUM_CAN][256];
    char csv[256];
    for (uint8_t i = 0; fgets(csv, 256, fp); i++) {
        strcpy(currentCAN[i], csv);
    }

    // Read values
    uint64_t fullData;
    uint8_t len = 0;
    sscanf(currentCAN[bus], "%x,%lx,%hhd", id, &fullData, &len);

    // Split hex data into bytes
    for (uint8_t i = 0; i < len; i++) {
        data[i] = (fullData >> (8 * (len-i-1))) & 0xFF;
    }

    // Clear entries from file
    freopen(FILE_NAME, "w", fp);

    // Re-write entry that wasn't read
    // Leave entry that was read as a blank line
    for (uint8_t i = 0; i < NUM_CAN; i++) {
            if (bus == i) {
                fprintf(fp, "\n");
            } 
            else {
                // As long as it wasn't empty or a blank line,
                // Print back data, otherwise, print newline
                if((strcmp(currentCAN[i], "\n")!=0) && (strcmp(currentCAN[i], "")!=0)){
                fprintf(fp, "%s", currentCAN[i]);
                }

                else{
                    fprintf(fp, "\n");
                }
            }
        }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
    return len;
}