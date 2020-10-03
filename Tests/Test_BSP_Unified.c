#include "common.h"
#include "config.h"
#include <bsp.h>

int testADC(void){
    BSP_ADC_Init(ADC_0);
    BSP_ADC_Init(ADC_1);

    char ch;
    // puts("flush input");
    while(1){
        printf("Accelerator: %5.1d\tBrake: %5.1d\r", 
            BSP_ADC_Get_Millivoltage(ADC_0), BSP_ADC_Get_Millivoltage(ADC_1));
    }
}

int testCAN(void){
    BSP_CAN_Init(CAN_1);
    BSP_CAN_Init(CAN_2);

    uint8_t txData[4];
    uint8_t rxData[4] = {0};

    // generate expected values and id
    uint32_t id = rand() % 0xFFF;
    for(int i = 0; i < 4; i++){
        txData[i] = rand() % 0xFF;
    }

    printf("Expected values to CAN\n");
    printf("ID: 0x%x\n", id);
    for(int i = 0; i < 4; i++){
        printf("0x%x ", txData[i]);
    }
    printf("\n");

    // write to can 
    BSP_CAN_Write(CAN_1, id, txData, 4);
    
    // actual read
    uint8_t len = BSP_CAN_Read(CAN_1, &id, rxData);
    printf("Actual values read from CAN\n");
    printf("ID: 0x%x length: %d\n", id, len);
    for(int i = 0; i < 4; i++){
        printf("0x%x ",rxData[i]);
    }
    printf("\n");
}

int testContactors(void){
    BSP_Contactors_Init(MOTOR);
    BSP_Contactors_Init(ARRAY);

    printf("If no error status then all test passed\n");

    BSP_Contactors_Set(MOTOR, OFF);
    BSP_Contactors_Set(ARRAY, OFF);
    if(BSP_Contactors_Get(MOTOR) != OFF){
        printf("Motor status error\n");
    }
    if(BSP_Contactors_Get(ARRAY) != OFF){
        printf("Contactor status error\n");
    }

    BSP_Contactors_Set(MOTOR, ON);
    BSP_Contactors_Set(ARRAY, OFF);
    if(BSP_Contactors_Get(MOTOR) != ON){
        printf("Motor status error\n");
    }
    if(BSP_Contactors_Get(ARRAY) != OFF){
        printf("Contactor status error\n");
    }

    BSP_Contactors_Set(MOTOR, OFF);
    BSP_Contactors_Set(ARRAY, ON);
    if(BSP_Contactors_Get(MOTOR) != OFF){
        printf("Motor status error\n");
    }
    if(BSP_Contactors_Get(ARRAY) != ON){
        printf("Contactor status error\n");
    }

    BSP_Contactors_Set(MOTOR, ON);
    BSP_Contactors_Set(ARRAY, ON);
    if(BSP_Contactors_Get(MOTOR) != ON){
        printf("Motor status error\n");
    }
    if(BSP_Contactors_Get(ARRAY) != ON){
        printf("Contactor status error\n");
    }
    
}

int testPrecharge(void){
    char board[6];
    char status[4];
    char* motor = "MOTOR";
    char* array = "ARRAY";
    char* on = "ON";
    char* off = "OFF";
    
    printf("Run in conjunction with simulate.py to verify prechagre board statuses");

    while(1){
        printf("Enter the board you wish to set\n");
        gets(board);
        printf("Enter the status you wish to set\n");
        gets(status);
        if(!strcmp(board, motor)){
            if(!strcmp(status,on)){
                printf("Writing to csv...\n");
                BSP_Precharge_Write(MOTOR_PRECHARGE, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_Write(MOTOR_PRECHARGE, OFF);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else if(!strcmp(board,array)){
            if(!strcmp(status,on)){
                BSP_Precharge_Write(ARRAY_PRECHARGE, ON);
            }
            else if(!strcmp(status,off)){
                BSP_Precharge_Write(ARRAY_PRECHARGE, OFF);
            }
            else{
                printf("unrecognized status\n");
            }
        }
        else{
            printf("unrecognized board\n");
        }
        usleep(1000);
    }
}

int testSwitches(void){
    //BSP_Switches_Init();
    printf("Run in conjunction with simulate.py and verify correct reads\n");
    char ch;
    ch = getchar(); 
    while(1){
        printf("LT: %d\n", BSP_Switches_Read(LT));
        printf("RT: %d\n", BSP_Switches_Read(RT));
        printf("HDLT: %d\n", BSP_Switches_Read(HDLT));
        printf("FWD_REV: %d\n", BSP_Switches_Read(FWD_REV));
        printf("HZD: %d\n", BSP_Switches_Read(HZD));
        printf("CRS_SET: %d\n", BSP_Switches_Read(CRS_SET));
        printf("CRS_EN: %d\n", BSP_Switches_Read(CRS_EN));
        printf("REGEN: %d\n", BSP_Switches_Read(REGEN));
        printf("IGN_1: %d\n", BSP_Switches_Read(IGN_1));
        printf("IGN_2: %d\n", BSP_Switches_Read(IGN_2));
    }
}

int testUART(void){
    BSP_UART_Init(UART_2);
    BSP_UART_Init(UART_3);

    while(1){
        float speed = (rand() % 500) / 10.0;
        int cruiseEn = rand() % 2;
        int cruiseSet = rand() % 2;
        int regenEn = rand() % 2;
        int CANerr = rand() % 10;
        
        char str[128];
        sprintf(str, "%f, %d, %d, %d, %d", speed, cruiseEn, cruiseSet, regenEn, CANerr);

        BSP_UART_Write(UART_2, str , 128);

        char out[2][128];
        BSP_UART_Read(UART_2, out[UART_2]);
        BSP_UART_Read(UART_3, out[UART_3]);
        out[UART_2][strlen(out[UART_2])-1] = 0; // remove the newline, so we can print both in one line for now
        out[UART_3][strlen(out[UART_3])-1] = 0;
        printf("UART 2: %s\tUART 3: %s\r", out[UART_2], out[UART_3]);
    }

}

int main(){

    // BSP_Timer_Init();

    // testADC();
    // testCAN();
    // testContactors();
    // testPrecharge();
    // testSwitches();
    testUART();

    return 0;
}


