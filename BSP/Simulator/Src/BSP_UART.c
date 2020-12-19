/* Copyright (c) 2020 UT Longhorn Racing Solar */

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
 * Encode the string so it's always printable
 * @param in input string
 * @param out output string
 * @return void
 * Note that strlen(output) == 2 * strlen(input), so ensure correct sizing
 */
static void convertTo(const uint8_t *in, char *out, int len) {
    for (int i=0; i<len; i++) {
        int hi = in[i] >> 4;
        int lo = in[i] & 0xf;
        out[i*2  ] = hi > 9 ? hi + 87 : hi + 48;
        out[i*2+1] = lo > 9 ? lo + 87 : lo + 48;
    }
    out[len*2] = '\0';
}

/**
 * Recover an encoded string
 * @param in the encoded string
 * @param out where to write the normal string
 * Note that strlen(input) == 2 * strlen(output)
 */
static void convertFrom(const char *in, char *out) {
    int len = strlen(in) / 2;
    for (int i=0; i<len; i++) {
        int hi = in[i*2  ] > 96 ? in[i*2  ] - 87 : in[i*2  ] - 48;
        int lo = in[i*2+1] > 96 ? in[i*2+1] - 87 : in[i*2+1] - 48; 

        out[i] = (hi << 4) | lo;
    }
    out[len] = '\0';
}

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */
void BSP_UART_Init(UART_t uart) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(UART_CSV);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reads a specific line of the CSV file 
 *          based on the UART device selected to 
 *          get the information from.
 * @pre     str should be at least 128bytes long.
 * @param   uart device selected
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
    char csv[NUM_UART][RX_SIZE*2];
    for (int i=0; fgets(csv[i], RX_SIZE, fp); i++);

    // Put string into return buffer
    convertFrom(csv[uart], str);

    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);

    return strlen(str);
}

/**
 * @brief   Writes a string to the CSV file in the 
 *          line respective to the UART device selected
 *          to write information to.
 * @param   uart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART_Write(UART_t uart, char* str, uint32_t len) {
    // Get current values in CSV
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("UART not available\n\r");
        return 0;
    }

    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Copying current contents
    char currentUART[NUM_UART][TX_SIZE*2];
    char csv[TX_SIZE*2];

    for(uint8_t i = 0; fgets(csv, TX_SIZE, fp); i++){
        strcpy(currentUART[i], csv);
    }

    // Put the converted string in variable for later
    convertTo(str, csv, len);

    // Close file
    flock(fno, LOCK_UN);
    fclose(fp);

    // Open to write
    fp = fopen(FILE_NAME, "w");
    
    // Lock file
    fno = fileno(fp);
    flock(fno, LOCK_EX);

    // Write to the file
    for(uint8_t i = 0; i < NUM_UART; i++){
        if(uart == i){
            fprintf(fp, "%s\n", csv); // write the converted string
        }else{
            fprintf(fp, "%s", currentUART[i]);
        }
        //fputc('\n', fp);
    }
    
    // Unlock file
    flock(fno, LOCK_UN);
    fclose(fp);

    return strlen(str);
}
