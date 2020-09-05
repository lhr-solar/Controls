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
/**
 * Moves the cursor n lines
 */
static void moveCursorUp(int n) {
    if (n < 1) n = 1;
    if (n > 99) n = 99;
    printf("%c[%dA", 0x1B, n);
}

static void timerTest(){
    printf("-----PLEASE RUN THIS TEST FILE IN CONJUCTION WITH THE GUI-----\n");
        //BSP_ADC TEST -----------------------------------------------------------
        //printf("--------------------------------------------------------------\n");
        printf("-------------------------ADC TEST-----------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("As you move the pedals their voltage values should change\n");
        printf("--------------------------------------------------------------\n");
        printf("Accelerator: %5.1d mV\tBrake: %5.1d mV\n", 
                BSP_ADC_Get_Millivoltage(ADC_0), BSP_ADC_Get_Millivoltage(ADC_1));

        //BSP_CAN TEST -----------------------------------------------------------
        //NOTE: The can test file has two possible test cases: CAN1 OR CAN2 and the
        // test file has them separated
        //TODO: Implement a variation of the CAN tests to unify both tests
        //printf("--------------------------------------------------------------\n");
        printf("-------------------------CAN TEST----------------------------\n");
        printf("--------------------------------------------------------------\n");

        //BSP_SWITCHES TEST ------------------------------------------------------
        //printf("--------------------------------------------------------------\n");
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
        printf("The UARTs are printing random values, UART 1 communicates with\n");
        printf("the gecko display while UART2 with the user for input and display\n");
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

        BSP_UART_Write(UART_1, str , TX_SIZE);
        BSP_UART_Write(UART_2, str1, TX_SIZE);

        char out[2][TX_SIZE];
        BSP_UART_Read(UART_1, out[UART_1]);
        BSP_UART_Read(UART_2, out[UART_2]);
        out[UART_1][strlen(out[UART_1])-1] = 0; // remove the newline, so we can print both in one line for now
        out[UART_2][strlen(out[UART_2])-1] = 0;

        printf("        SPEED  CRS_EN  CRS_SET REGEN CAN_ERROR\n");
        printf("UART 1: %s\n", out[UART_1]);
        printf("UART 2: %s\n", out[UART_2]);

        //BSP_Contactors -----------------------------------------------------
        //NOTE: The contactors test file requires input from the user
        //TODO: Implement a variation of the contactors test in here
        //printf("--------------------------------------------------------------\n");
        printf("----------------------CONTACTORS TEST-------------------------\n");
        printf("--------------------------------------------------------------\n");
        printf("Contactors state is constatly updated, changes visible in GUI\n");
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

        // Moving cursor back up
        moveCursorUp(23);
        
        // Erasing lines 
        /*for(int i = 0; i < 27; i++){
            printf("                                                                              \n");
        }
        // Move cursor back up
        moveCursorUp(27);*/
        
}

int main() {
    BSP_ADC_Init(ADC_0);
    BSP_ADC_Init(ADC_1);

    BSP_Contactors_Init(MOTOR);
    BSP_Contactors_Init(ARRAY);

    BSP_UART_Init();

    BSP_Switches_Init();
    
    BSP_Timer_Init(20, timerTest, TIMER_1);

    while(1) {
        //BSP_Timer TEST ---------------------------------------------------------
        BSP_Timer_Update();
    }
    printf("\n");
    return 0; 
}
