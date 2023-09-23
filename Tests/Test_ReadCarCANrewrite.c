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
    printf("\n\r");
    printf("\r\nArray Ignition Status    : %s", ((ArrayIgnitionStatus_Get()) ? "ON" : "OFF"));
    printf("\r\nCharge Message Saturation: %d", ChargeMsgSaturation_Get());
    printf("\r\nThreshold                : %s", ((ChargeMsgSaturation_Get() >= 7.5) ? "Threshold reached" : "Threshold not reached"));
    printf("\r\nCharge Enable            : %s", (ChargeEnable_Get() ? "TRUE" : "FALSE"));
    printf("\r\nArray Contactor          : %s", ((Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR) == ON) ? "ON" : "OFF"));    
    printf("\r\nPrecharge Complete?      : %s", ((PreChargeComplete_Get()) ? "Yes" : "No"));    
    printf("\r\nMotor Contactor          : %s", ((Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR) == ON) ? "ON" : "OFF")); 
}

Minion_Error_t mErr;
static void turnIgnitionON(){
    printf("\n\r=========== Testing: Ignition ON with Charge Enable Messages ===========");
        printf("\n\r=========== Expected output: One message with Array Contactor turned ON ===========");
        Minion_Write_Output(IGN_1, 1, &mErr);                      // Ignition arr ON
        Minion_Write_Output(IGN_2, 0, &mErr);                      // Ignition motor OFF
        while(Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR) != ON){ 
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
            //infoDump();
        }
    infoDump();
}


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


static void sendArrayEnableMsg(){
    printf("\n\r=========== Array Enable Msg Sent ===========");
        while(Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR) != ON){ 
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
}

static void sendMotorControllerEnableMsg(){
    printf("\n\r=========== Motor Controller Enable Msg Sent ===========");
        while(Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR) != ON){ 
            CANbus_Send(all_clear_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
}

static void sendDisableMsg(){
    printf("\n\r=========== Motor Controller Disable Msg Sent ===========");
        while(Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR) != ON){ 
            CANbus_Send(disable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
        }
}


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

                turnIgnitionON(); // Helper to turn ignition on

                // Test case for when ignition is OFF but charge enable messages are read
                // Info dumped to show that message threshold is reached (and maxed out) but contactor is consistently off
                printf("\n\r");
                printf("\n\r=========== Testing: Ignition OFF with Charge Enable Messages ===========");
                printf("\n\r=========== Expected output: Array contactor always OFF ===========");
                Minion_Write_Output(IGN_1, false, &mErr); // Ignition OFF
                for(int i = 0; i < ARBITRARY_LOOP_NUM; i++){ 
                    CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN); // Charge enable messages
                    infoDump(); 
                }

                turnIgnitionON(); // Helper to turn ignition on
                
                // Test case for when ignition is OFF and charge disable messages are read
                // Info dumped to show that message threshold decreases and contactor is consistently off
                printf("\n\r");
                printf("\n\r=========== Testing: Ignition OFF with Charge Disable Messages ===========");
                printf("\n\r=========== Expected output: Threshold goes down, array contactor always OFF===========");
                Minion_Write_Output(IGN_1, false, &mErr); // Ignition OFF
                for(int i = 0; i < ARBITRARY_LOOP_NUM; i++){   
                    CANbus_Send(charge_disable_msg, CAN_BLOCKING, CARCAN); // Charge disable messages
                    infoDump();
                }

                turnIgnitionON(); // Helper to turn ignition on

                // Test case for when ignition is ON but charge disable messages are read
                // Info dumped shows that message threshold increases but contactor is consistently off
                printf("\n\r");
                printf("\n\r=========== Testing: Ignition ON with Charge Disable Messages ===========");
                printf("\n\r=========== Expected output: Threshold increases, array contactor always OFF ===========");
                Minion_Write_Output(IGN_1, true, &mErr); // Ignition OFF
                for(int i = 0; i < ARBITRARY_LOOP_NUM; i++){  
                    CANbus_Send(charge_disable_msg, CAN_BLOCKING, CARCAN); // Charge disable messages
                    infoDump();
                }

                turnIgnitionON(); // Helper to turn ignition on

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
                    CANbus_Send(charge_disable_msg, CAN_BLOCKING, CARCAN); // Charge disable messages
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