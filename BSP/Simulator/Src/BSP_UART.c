/**
 * Source file to mimic UART communication
 * that will be used to communicate with the
 * Gecko display board
 */ 

#include "BSP_UART.h"

#define FILE_NAME DATA_PATH(UART_CSV)
#define RX_SIZE 64
#define TX_SIZE 128

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */
void BSP_UART_Init(void) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(UART_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads one line from the CSV file
 * @pre     str should be at least 128bytes long.
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(UART_t uart, char* str) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("UART not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Get raw CSV strings
    char csv[NUM_UART][RX_SIZE];
    for (int i=0; fgets(csv[i], RX_SIZE, fp); i++);

    // Put string into return buffer
    strcpy(str, csv[uart]);

    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);

    return strlen(str);
}

/**
 * @brief   Writes a string to the CSV file
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  numer of bytes that were sent
 */
uint32_t BSP_UART_Write(char* str, uint32_t len) {
    FILE* fp = fopen(FILE_NAME, "w");
    if (!fp) {
        printf("UART not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Write to the file
    fprintf(fp, "%s", str);

    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);

    return strlen(str);
}
