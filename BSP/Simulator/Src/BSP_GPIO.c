/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Source file to mimic GPIO port communication
 */ 

#include "BSP_GPIO.h"

#define FILE_NAME DATA_PATH(GPIO_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   port to initialize
 * @param	mask - pins
 * @param	direction - input or output
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, direction_t direction) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(GPIO_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads value of the line of
 *          the CSV file
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("GPIO not available\n\r");
        return 0;
    }
    
    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    uint16_t csvContents[NUM_PORTS] = {0};

    int tok;
    for (int i = 0; i < NUM_PORTS && fscanf(fp, "%d", &tok); i++) {
        csvContents[i] = tok;
    }

    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);

    return csvContents[port];
}

/**
 * @brief   Writes data to a line in CSV fle
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        perror("GPIO not available\n\r");
        exit(EXIT_FAILURE);
    }
    
    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    int csvContents[NUM_PORTS] = {0};

    for(int i = 0; i < NUM_PORTS; i++){
        int tok;
        if (fscanf(fp, "%d", &tok) != EOF) {
            csvContents[i] = tok;
        }
    }

    csvContents[port] = data;

    freopen(FILE_NAME, "w", fp);

    for(int i = 0; i < NUM_PORTS; i++){
        fprintf(fp, "%d\n", csvContents[i]);
    }

    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);
}