/**
 * Source file to mimic UART communication
 * that will be used to communicate with the
 * Gecko display board
 */ 

#include "BSP_UART.h"

#define FILE_NAME DATA_PATH(UART_CSV)

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
 * @brief   Gets one line of ASCII text that was received.
 * @pre     str should be at least 128bytes long.
 * @param   str : pointer to buffer to store the string. This buffer should be initialized
 *                  before hand.
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(char *str) {

}

/**
 * @brief   Transmits data to through UART line
 * @param   str : pointer to buffer with data to send.
 * @param   len : size of buffer
 * @return  numer of bytes that were sent
 */
uint32_t BSP_UART_Write(char *str, uint32_t len) {

}
