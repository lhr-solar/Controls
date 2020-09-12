#include "BSP_Lights.h"


#define FILE_NAME DATA_PATH(LIGHTS_CSV)

void BSP_Lights_Init(void){
    if(access(FILE_NAME, F_OK) != 0){
        // File doesn't exist if true
        perror(LIGHTS_CSV);
        exit(EXIT_FAILURE);
    }
}

state_t BSP_Lights_Read(LIGHT_t LightChannel){
    FILE* fp = fopen(FILE_NAME, "r");
    //will this error case ever be reached? Init function won't fire if file is unavailable
    if (!fp) {
        printf("Lights not available\n\r");
        return 2;
    }

    //Exclusive lock the file open
    flock((fileno(fp)), LOCK_EX);

    //get raw CSV string 
    char csv[16];
    fgets(csv,16,fp);

    //convert to integer
    uint16_t switchInt = atoi(csv);

    //unlock file
    flock((fileno(fp)), LOCK_UN);
    fclose(fp);

    //bitwise AND with CSV integer and 1 at the bit position of the selected light
    if(switchInt & (1<<LightChannel)){
        return ON;
    } else {
        return OFF;
    }


}
void BSP_Lights_Switch(LIGHT_t LightChannel){
    FILE* fp = fopen(FILE_NAME, "r");
    //will this error case ever be reached? Init function won't fire if file is unavailable
    if (!fp) {
        printf("Lights not available\n\r");
        return 2;
    }

    //Exclusive lock the file open
    flock((fileno(fp)), LOCK_EX);

    //get data in file
    char csv[16];
    fgets(csv,16,fp);

    //convert to integer
    uint16_t switchInt = atoi(csv);

    //switch from low->high/high->low using XOR
    switchInt = switchInt^(1<<LightChannel);

    //unlock + close file
    flock((fileno(fp)), LOCK_UN);
    fclose(fp);

    //reopen cleared file
    FILE* fp1 = fopen(FILE_NAME, "w");
    //Exclusive lock the file open
    flock((fileno(fp1)), LOCK_EX);

    //enter updated data string
    fprintf(fp1,"%d",switchInt);

    //unlock + close file
    flock((fileno(fp1)), LOCK_UN);
    fclose(fp1);

}