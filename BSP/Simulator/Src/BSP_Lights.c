#include "BSP_Lights.h"


#define FILE_NAME DATA_PATH(LIGHTS_CSV)

void BSP_Lights_Init(void){
    if(access(FILE_NAME, F_OK) != 0){
        // File doesn't exist if true
        perror(LIGHTS_CSV);
        exit(EXIT_FAILURE);
    }
}

void BSP_Lights_Set(){

}
