/**
 * Test file that unifies all individual test files of the BSP module
 * 
 * Run this test in conjunction with the simulator 
 * GUI. 
 * 
 */ 

#include "common.h"
#include "config.h"
#include <unistd.h>
#include "BSP_Timer.h"
#include "BSP_ADC.h"
#include <bsp.h>

#define TX_SIZE 128
#define LEN 4
/**
 * Moves the cursor n lines
 */
static void moveCursorUp(int n) {
    if (n < 1) n = 1;
    if (n > 99) n = 99;
    printf("%c[%d;%dH", 0x1B, 1, 1);
}

int main() {

    BSP_ADC_Init(ADC_0);
    BSP_ADC_Init(ADC_1);

    BSP_CAN_Init(CAN_1);
    //BSP_CAN_Init(CAN_2);

    BSP_Contactors_Init(MOTOR);
    BSP_Contactors_Init(ARRAY);

    BSP_UART_Init(UART_3);
    BSP_UART_Init(UART_2);

    BSP_Switches_Init();

    uint8_t txData[LEN] = {0x00, 0x12, 0x42, 0x5A};

    uint8_t txData2[LEN] = {0};
    
    uint32_t id2 = 0x321;

    bool negativeAcc = false;

    int threshold = 1000 * 10;
    int writevsread = threshold;

    while(1) {
        printf("%c[%d;%dH", 0x1B, 1, 1);
        printf("-----PLEASE RUN THIS TEST FILE IN CONJUCTION WITH THE GUI-----\n");
        //BSP_ADC TEST -----------------------------------------------------------
        printf("-------------------------ADC TEST-----------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("As you move the pedals in the GUI their voltage values should change\n");
        printf("Accelerator: %5.1d mV\tBrake: %5.1d mV\n", 
                BSP_ADC_Get_Millivoltage(ADC_0), BSP_ADC_Get_Millivoltage(ADC_1));

        //BSP_CAN TEST -----------------------------------------------------------
        //NOTE: The can test file can test both CAN1 ot CAN2 and
        // it has them in separate functions
        printf("-------------------------CANs TEST----------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("CAN bus 1 sends an ID and Message, which can be seen in the GUI\n");
        // CAN 1 TEST - BASICALLY JUST COPIED IT FROM BSP_CAN.c
        if(txData[3] > 0x70){
            negativeAcc = true; 
        }else if(txData[3] < 0x40){
            negativeAcc = false;
        }

        if(!negativeAcc){
            txData[3] = txData[3]+1;
        }else{
            txData[3] = txData[3]-1;
        }

        uint32_t id;
        BSP_CAN_Write(CAN_1, 0x221, txData, LEN);
        
        uint8_t rxData[LEN] = {0};
        uint8_t len = BSP_CAN_Read(CAN_1, &id, rxData);
        printf("ID: 0x%x\nData: ", id);
        for (uint8_t i = 0; i < len; i++) {
            printf("0x%x ", rxData[i]);
            fflush(stdout);
        }
        printf("\n");

        // CAN 2 test by random values
        uint8_t rxData2[LEN] = {0};

        if(writevsread == threshold){
            writevsread = 0;
            id2 = rand() % 0xFFF;
            for(int i = 0; i < LEN; i++){
                txData2[i] = rand() % 0xFF;
            }
        } else {
            writevsread++;
        }

        BSP_CAN_Write(CAN_2, id2, txData2, LEN);

        printf("Expected values to CAN_2\n");
        printf("ID: 0x%x\n", id2);
        for(int i = 0; i < LEN; i++){
            printf("0x%x ", txData2[i]);
            fflush(stdout);
        }
        printf("\n");

        int can2len = BSP_CAN_Read(CAN_2, &id2, rxData2);
        printf("Actual values read from CAN_2\n");
        printf("ID: 0x%x length: %d\n", id2, can2len);
        for(int i = 0; i < can2len; i++){
            printf("0x%x ",rxData2[i]);
            fflush(stdout);
        }
        printf("\n");

        //BSP_SWITCHES TEST ------------------------------------------------------
        printf("---------------------- SWITCHES TEST--------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("Press switches on the GUI, their state will be displayed:\n");
        printf("LT\tRT     HDLT   FWD_REV   HZD   CRS_SET CRS_EN  REGEN   IGN_1  IGN_2\n");
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", 
                BSP_Switches_Read(LT),
                BSP_Switches_Read(RT),
                BSP_Switches_Read(HDLT),
                BSP_Switches_Read(FWD_REV),
                BSP_Switches_Read(HZD),
                BSP_Switches_Read(CRS_SET),
                BSP_Switches_Read(CRS_EN),
                BSP_Switches_Read(REGEN),
                BSP_Switches_Read(IGN_1),
                BSP_Switches_Read(IGN_2));

        //BSP_UART TEST ----------------------------------------------------------
        //printf("--------------------------------------------------------------\n");
        printf("-------------------------UART TEST----------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("The UARTs are showing random values, UART 1 communicates with\n");
        printf("the gecko display while UART2 with the user direclty \n");
        float speed = (rand() % 500) / 10.0;
        int cruiseEn = rand() % 2;
        int cruiseSet = rand() % 2;
        int regenEn = rand() % 2;
        int CANerr = rand() % 10;
        char str[TX_SIZE];
        sprintf(str, "%f, %d, %d, %d, %d", speed, cruiseEn, cruiseSet, regenEn, CANerr);

        float speed1 = (rand() % 500) / 10.0;
        int cruiseEn1 = rand() % 2;
        int cruiseSet1 = rand() % 2;
        int regenEn1 = rand() % 2;
        int CANerr1 = rand() % 10;
        char str1[TX_SIZE];
        sprintf(str1, "%f, %d, %d, %d, %d", speed1, cruiseEn1, cruiseSet1, regenEn1, CANerr1);

        BSP_UART_Write(UART_3, str , TX_SIZE);
        BSP_UART_Write(UART_2, str1, TX_SIZE);

        char out[2][TX_SIZE];
        BSP_UART_Read(UART_3, out[UART_3]);
        BSP_UART_Read(UART_2, out[UART_2]);
        out[UART_3][strlen(out[UART_3])-1] = 0; // remove the newline, so we can print both in one line for now
        out[UART_2][strlen(out[UART_2])-1] = 0;

        printf("        SPEED  CRS_EN  CRS_SET REGEN CAN_ERROR\n");
        printf("UART 1: %s\n", out[UART_3]);
        printf("UART 2: %s\n", out[UART_2]);

        //BSP_Contactors -----------------------------------------------------
        //NOTE: The contactors test file requires input from the user in the original test
        printf("----------------------CONTACTORS TEST-------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("Contactors are constantly updated randomly, changes also visible in GUI\n");
        int motorOrArray = rand() % 2;
        int onOrOff = rand() % 2;                                                                                                           
        if (motorOrArray == 1){
            if (onOrOff == 1){
                BSP_Contactors_Set(MOTOR, ON);
            }
            else {
                BSP_Contactors_Set(MOTOR, OFF);  
            }
        }else{
            if (onOrOff == 1){
                BSP_Contactors_Set(ARRAY, ON);
            }
            else {
                BSP_Contactors_Set(ARRAY, OFF);
            }
        }
        printf("MOTOR CONTACTOR STATE: %d\n", BSP_Contactors_Get(MOTOR));
        printf("ARRAY CONTACTOR STATE: %d\n", BSP_Contactors_Get(ARRAY));

        //BSP_Precharge -----------------------------------------------------
        //NOTE: The contactors test file requires input from the user in the original test
        printf("----------------------Precharge TEST--------------------------\n");
        printf("--------------------------------------------------------------\n");
        uint8_t precCase = rand() % 3;

        switch (precCase)
        {
        case 0:
            BSP_Precharge_Write(MOTOR, OFF);
            printf("MOTOR PRECHARGE STATE SET TO: OFF\n");
            break;
        case 1:
            BSP_Precharge_Write(MOTOR, ON);
            printf("MOTOR PRECHARGE STATE SET TO: ON \n");
            break;
        case 2:
            BSP_Precharge_Write(ARRAY, OFF);
            printf("ARRAY PRECHARGE STATE SET TO: OFF\n");
            break;
        case 3:
            BSP_Precharge_Write(ARRAY, ON);
            printf("ARRAY PRECHARGE STATE SET TO: ON \n");
            break;
        default:
            break;
        }

        // Moving cursor back up
        //moveCursorUp(40);
        //break;
    }
    printf("\n");
    return 0; 
}
