#include "BSP_GPIO.h"

#define FILE_NAME DATA_PATH(GPIO_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and throws
 *          an error if not
 * @param   None
 * @return  None
 */ 
void BSP_GPIO_Init(void) {
    if (access(FILE_NAME, F_OK) != 0) {
        // File doesn't exist if true
        perror(GPIO_CSV);
        exit(EXIT_FAILURE);
    }
}

int BSP_GPIO_Read(Pin pn){
    FILE* fp = fopen(FILE_NAME, "r");
    if(!fp){
        perror("GPIO file was not found\n");
        exit(EXIT_FAILURE);
    }
    
    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    int csvContents[GPIOSIZE];

    for(int i = 0; i < GPIOSIZE; i++){
        int tok;
        fscanf(fp, "%d", &tok);
        if(tok){
            csvContents[i] = tok;
        } else {
            csvContents[i] = -1;
        }
    }

    // for(int i = 0; i < GPIOSIZE; i++){
    //     printf("%d\n", csvContents[i]);
    // }

    fclose(fp);

    return csvContents[pn];
}

int BSP_GPIO_Write(Pin pn, int data){
    FILE* fp = fopen(FILE_NAME, "r");
    if(!fp){
        perror("GPIO file was not found\n");
        exit(EXIT_FAILURE);
    }
    
    // Lock file
    int fno = fileno(fp);
    flock(fno, LOCK_EX);

    int csvContents[GPIOSIZE];

    for(int i = 0; i < GPIOSIZE; i++){
        int tok;
        fscanf(fp, "%d", &tok);
        if(tok){
            csvContents[i] = tok;
        } else {
            csvContents[i] = -1;
        }
    }

    csvContents[pn] = data;

    freopen(FILE_NAME, "w", fp);

    for(int i = 0; i < GPIOSIZE; i++){
        fprintf(fp, "%d\n", csvContents[i]);
    }

    // for(int i = 0; i < GPIOSIZE; i++){
    //     printf("%d\n", csvContents[i]);
    // }

    fclose(fp);

    return 1;

}