/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Source file for the library to interact
 * over SPI with minion board(s)
 */

#include "BSP_SPI.h"

#define FILE_NAME DATA_PATH(SPI_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */
void BSP_SPI_Init(void) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(CAN_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Writes a message to the SPI CSV file
 * @param   txBuf data to transmit
 * @param   txLen length of the data packet
 * @return  None
 */
void BSP_SPI_Write(uint8_t* txBuf, uint8_t txLen) {
    FILE* fp = fopen(FILE_NAME, "w");
    if (!fp) {
        printf("SPI not available\n\r");
        return;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Write data
    for (uint8_t i = 0; i < txLen; i++) {
        fprintf(fp, "%x", txBuf[i]);
    }

    // CLose file
    flock(fno, LOCK_UN);
    fclose(fp);
}

/**
 * @brief   Receives a message through SPI
 * @param   rxBuf buffer to store the received data
 * @param   rxLen length of the buffer
 * @return  None
 */
void BSP_SPI_Read(uint8_t* rxBuf, uint8_t rxLen) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("SPI not available\n\r");
        return;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Read file
    char csv[128];
    uint64_t fullData;
    fgets(csv, 128, fp);
    sscanf(csv, "%x", &fullData);

    // Split hex data into bytes
    for (uint8_t i = 0; i < rxLen; i++) {
        rxBuf[i] = (fullData >> (8 * (len-i-1))) & 0xFF;
    }

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);
}
