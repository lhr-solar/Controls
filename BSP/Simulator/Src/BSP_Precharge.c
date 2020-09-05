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
void BSP_Precharge_write(board_t board, state_t status){
    FILE* fp = fopen(FILE_NAME, "r+");

    if(!fp){
        printf("Precharge file is not found\n");
        return;
    }

    int data;
    fscanf(fp,"%d", &data);

    // update bit of data accordingly
    int mask = (1 << board);
    data = data & (~mask);
    data = data | (status << board);

    // only single value should be kept in csv file
    fprintf(fp, "%d", data);

    fclose(fp);
}






