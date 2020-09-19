#include "BSP_Precharge.h"

#define FILE_NAME DATA_PATH(PRECHARGE_CSV)

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void BSP_Precharge_Write(board_t board, state_t status){
    // first get previous board states from csv file
    FILE* fp = fopen(FILE_NAME, "r");

    if(!fp){
        printf("Precharge file is not found\n");
        return;
    }

    int fno = fileno(fp);
    flock(fno,LOCK_EX);

    int data;
    fscanf(fp,"%d", &data);
    // update bit of data accordingly

    int mask = (1 << board);
    data = data & (~mask);
    data = data | (status << board);

    flock(fno,LOCK_UN);
    fclose(fp);

    // only single value should be kept in csv file
    // this part is to write data after read to clean csv file
    fp = fopen(FILE_NAME, "w");

    if(!fp){
        printf("Precharge file is not found\n");
        return;
    }

    fno = fileno(fp);
    flock(fno,LOCK_EX);

    fprintf(fp, "%d", data);

    flock(fno,LOCK_UN);
    fclose(fp);
}






