/**
 * 
 * This file tests ignition and charge enable interactions.
 * 
 *
 * This test is run in LoopBack mode with all messages sent and received by the LeaderBoard.
 * However, it can be run in conjunction with motor-sim and car-sim
 * (which don't do anything) when simulated to appease Renode.
 * 
 * @file 
 * @author Diya Rajon (drajon@utexas.edu)
 * @brief Tests read car can structure
 * @version 
 * @date 2023-7-14
 *
 * @copyright Copyright (c) 2022 Longhorn Racing Solar
 *
 */

#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "ReadCarCAN.h"
#include "Contactors.h"
#include "CANConfig.h"
#include "Minions.h"
#include "UpdateDisplay.h"
#include "Display.h"
#include "common.h"

enum { // Test menu enum
    TEST_RENODE, 
    TEST_HARDWARE_CHARGE_ENABLE,
    TEST_HARDWARE_CHARGE_DISABLE
};

/*** Constants ***/
#define TEST_OPTION TEST_RENODE // Decide what to test based on test menu enum

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

#define SATURATION_THRESHOLD_TEST (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 
#define ARBITRARY_LOOP_NUM 5

static CANDATA_t bps_trip_msg = {.ID=BPS_TRIP, .idx=0, .data={1}};
static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={100}};
static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={0}};
static CANDATA_t charge_enable_msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b01}};
static CANDATA_t disable_msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b00}};
static CANDATA_t all_clear_enable_msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b10}};
static CANDATA_t enable_msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b11}};

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

static void infoDump(){
    OS_ERR err;
    OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);
    // printf("\n\r");
    printf("\r\nArray Contactor          : %s", ((Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR) == ON) ? "ON" : "OFF")); 
     printf("\r\nArray Ignition Status    : %s", ((ArrayIgnitionStatus_Get()) ? "ON" : "OFF"));
     printf("\r\nCharge Message Saturation: %d", ChargeMsgSaturation_Get());
     printf("\r\nThreshold                : %s", ((ChargeMsgSaturation_Get() >= 7.5) ? "Threshold reached" : "Threshold not reached"));
     printf("\r\nCharge Enable            : %s", (ChargeEnable_Get() ? "TRUE" : "FALSE"));   
     printf("\r\nPrecharge Complete?      : %s", ((PreChargeComplete_Get()) ? "Yes" : "No"));    


    printf("\n\r");
    printf("\r\nMotor Contactor          : %s", ((Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR) == ON) ? "ON" : "OFF"));
    printf("\r\nMotor Ignition Status    : %s", ((MotorControllerIgnition_Get()) ? "ON" : "OFF"));
    printf("\r\nCharge Message Saturation: %d", PlusMinusMsgSaturation_Get());
    printf("\r\nThreshold                : %s", ((PlusMinusMsgSaturation_Get() >= 7.5) ? "Threshold reached" : "Threshold not reached"));
    printf("\r\nPrecharge Complete?      : %s", ((PreChargeComplete_Get()) ? "Yes" : "No"));    
    printf("\n\r");
    printf("\n\r");
}


Minion_Error_t mErr;



static void turnIgnitionToMotorON(){
    printf("\n\r=========== Turn Ignition to Motor ===========");
    Minion_Write_Output(IGN_2, 1, &mErr);                      // Ignition motor ON
    Minion_Write_Output(IGN_1, 0, &mErr);                      // Ignition array OFF
}

static void turnIgnitionToArrayON(){
    printf("\n\r=========== Turn Ignition to Array ===========");
    Minion_Write_Output(IGN_2, 0, &mErr);                      // Ignition motor OFF
    Minion_Write_Output(IGN_1, 1, &mErr);                      // Ignition array ON
}

static void turnIgnitionOFF(){
    printf("\n\r=========== Turn Ignition to OFF ===========");
    Minion_Write_Output(IGN_2, 0, &mErr);                      // Ignition motor OFF
    Minion_Write_Output(IGN_1, 0, &mErr);                      // Ignition array OFF
}


static void sendArrayEnableMsg(uint8_t isIgnitionOn){
    printf("\n\r=========== Array Enable Msg Sent ===========");
    if(isIgnitionOn == 1){
        while(!Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR)){
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN); 
            }   
     }else{
       //  for(int i = 0; i < 1; i++){
             CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        // }
     }
}


static void sendMotorControllerEnableMsg(uint8_t isIgnitionOn){
    printf("\n\r=========== Motor Controller Enable Msg Sent ===========");
    if(isIgnitionOn == 2){
        while(Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR) != true){
            CANbus_Send(all_clear_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
    }else{
        for(int i = 0; i < 1; i++){
            CANbus_Send(all_clear_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
    }
}


static void sendDisableMsg(){
    printf("\n\r=========== Motor Controller Disable Msg Sent ===========");
    CANbus_Send(disable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
}

static void sendEnableMsg(int isIgnitionOn){
    printf("\n\r=========== Enable Msg Sent ===========");
        if(isIgnitionOn == 2){
        while(!Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR)){
            CANbus_Send(enable_msg, CAN_BLOCKING, CARCAN); 
            }      // Charge Enable messages

        if (isIgnitionOn == 1){
            while(!Contactors_Get((ARRAY_BYPASS_PRECHARGE_CONTACTOR))){
            CANbus_Send(enable_msg, CAN_BLOCKING, CARCAN); 
            }      // Charge Enable messages
        }

        for(int i = 0; i < 1; i++){
            CANbus_Send(enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
    }
}

static void turnContactorOn(uint8_t isIgnitionOn){
    if(isIgnitionOn == 1){
        turnIgnitionToArrayON();
        sendArrayEnableMsg(1);
    }else if(isIgnitionOn == 2){
        turnIgnitionToMotorON();
        sendMotorControllerEnableMsg(2);
    }
}

// static void turnBothContactorsOn(){
//     turnContactorOn(1);
//     turnContactorOn(2);
// }

void Task1(){
    OS_ERR err;

    CPU_Init();
    Minion_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    Contactors_Init();
    CANbus_Init(CARCAN, NULL, 0);
    Display_Init();
    UpdateDisplay_Init();
    BSP_UART_Init(UART_2);
    
    OSTaskCreate(
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)TASK_READ_CAR_CAN_PRIO,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE/10,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    

    while(1){

        switch (TEST_OPTION) {
            case TEST_RENODE:
            // turnContactorOn(2); 
            // turnIgnitionOFF();
            // sendDisableMsg();
            // infoDump();  

            // turnContactorOn(1);
            // turnIgnitionOFF(); 
            // sendArrayEnableMsg(0);
            // infoDump();  

            // turnContactorOn(2); 
            // turnIgnitionOFF();
            // sendMotorControllerEnableMsg(0);
            // infoDump(); 

            turnContactorOn(2); 
            turnIgnitionOFF();
            sendEnableMsg(0);
            infoDump(); 

            turnIgnitionToArrayON();
            sendDisableMsg();
            infoDump();

            turnIgnitionToArrayON();
            sendArrayEnableMsg(1);
            infoDump();

            turnIgnitionToArrayON();
            sendMotorControllerEnableMsg(1);
            infoDump();

            turnIgnitionToArrayON();
            sendEnableMsg(1);
            infoDump();




            sendEnableMsg(2);
            infoDump();  
            sendEnableMsg(1);
            infoDump();      
            turnIgnitionOFF();
            sendDisableMsg(0);
            infoDump();

            turnContactorOn(1);    


            turnIgnitionOFF();
            sendArrayEnableMsg(0);
            infoDump();



            sendEnableMsg(0);



                // Test case for supply voltage
                printf("\n\r");
                printf("\n\r=========== Testing: Supply Voltage ===========");
                    CANbus_Send(supp_voltage_msg, CAN_BLOCKING, CARCAN); // Supply Voltage message
                    printf("Supply Voltage: %ld", SBPV_Get());
                
                // Test case for supply voltage
                printf("\n\r");
                printf("\n\r=========== Testing: State of Charge ===========");
                    *(uint64_t*)(&state_of_charge_msg.data) = 42000000;
                    CANbus_Send(state_of_charge_msg, CAN_BLOCKING, CARCAN); // State of Charge message
                    printf("State of Charge: %d",SOC_Get());
                

                printf("\n\n\rtest done, nothing should print after bps trip\r");

                // Test case for BPS Trip
                printf("\n\r");
                printf("\n\r=========== Testing: BPS Trip ===========");
                    CANbus_Send(bps_trip_msg, CAN_BLOCKING, CARCAN); // BPS Trip
                    infoDump();
                

                break;

            case TEST_HARDWARE_CHARGE_ENABLE:
                while(1){
                    CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN); // Charge enable messages
                    infoDump();
                }
                break;

            case TEST_HARDWARE_CHARGE_DISABLE:
                while(1){
                    CANbus_Send(disable_msg, CAN_BLOCKING, CARCAN); // Charge disable messages
                    infoDump();
                }
                break;
        }

    }
}

int main(){
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)13,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);

    while(1){};
}