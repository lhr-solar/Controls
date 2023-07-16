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
 * @version 2
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
#include "Display.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"
#include "Minions.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

#define SATURATION_THRESHOLD_TEST (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

static uint8_t supp_voltage_can_data = 0;
//static uint32_t state_of_charge_can_data = 0;

static CANDATA_t bps_trip_msg = {.ID=BPS_TRIP, .idx=0, .data={1}};
static CANDATA_t charge_enable_msg = {.ID=CHARGE_ENABLE, .idx=0, .data={1}};
static CANDATA_t charge_disable_msg = {.ID=CHARGE_ENABLE, .idx=0, .data={0}};
static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={0}};
static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={100}};

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

static void print_array_status(){
    if (Contactors_Get(ARRAY_CONTACTOR) == ON){
        printf("\r\nArray is ON\r\n");
    }
    else{
        printf("\r\nArray is OFF\n");
    }
}

static void print_threshold_status(){
    if (chargeMsgSaturation >= 7.5){
        printf("\r\nThreshold has been reached\n");
    }
    else{
        printf("\r\nThreshold was not reached\n");
    }
}

static void info_dump(){
    printf("\r\nCharge Message Saturation:   %d", chargeMsgSaturation);
    printf("\r\nCharge Enable            :   %d", ChargeEnable_Get());
    print_threshold_status();
    print_array_status();
}

void Task1(){
    OS_ERR err;

    CPU_Init();
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
        (OS_PRIO)3,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    Minion_Error_t mErr;

    while(1){
        
        printf("\n\r=========== Testing: Ignition ON ===========");
        printf("\n\r=========== Expected: output: Array contactor ON when threshold is reached ===========");
        for(int i = 0; i < 10; i++){ 
            Minion_Write_Output(IGN_1, ON, &mErr);                      // Ignition ON
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);       // Charge Enable messages
            info_dump();
        }

        printf("\n\r=========== Testing Ignition OFF ===========");
        printf("\n\r=========== Expected output: Array contactor always OFF ===========");
        for(int i = 0; i < 10; i++){ 
            Minion_Write_Output(IGN_1, OFF, &mErr);                     // Ignition OFF
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);       // Charge enable messages
            info_dump();
        }
        

        printf("\n\r=========== Testing: Threshold Increase ===========");
        printf("\n\r=========== Expected output: Threshold increases ===========");
        for(int i = 0; chargeMsgSaturation <= SATURATION_THRESHOLD_TEST; i++){  // prints until max saturation is reached
            CANbus_Send(charge_enable_msg, CAN_BLOCKING, CARCAN);               // Charge enable messages
            info_dump();
        }

        printf("\n\r=========== Testing: Threshold ===========");
        printf("\n\r=========== Expected output: Threshold goes down ===========");
        for(int i = 0; chargeMsgSaturation >= -1*SATURATION_THRESHOLD_TEST; i++){    // prints until max negative saturation is reached
            CANbus_Send(charge_disable_msg, CAN_BLOCKING, CARCAN);                   // Charge disable messages
            info_dump();
        }
            
        printf("\n\r=========== Testing Supp Voltage ===========");
        for(int i = 0; i < 3; i++){
            supp_voltage_can_data += 10;
            supp_voltage_msg.data[0] = supp_voltage_can_data;
            CANbus_Send(supp_voltage_msg, CAN_BLOCKING, CARCAN);
            printf("\r\nSupplemental Voltage: %ld", SBPV);
        }
        
        printf("\r\n"); 
        printf("\r\n"); 
        printf("\r\n"); 

        printf("\n\r=========== Testing State of Charge ===========");
        for(int i = 0; i < 3; i++){
          //  state_of_charge_can_data += 100000;
        //    state_of_charge_msg.data[0] = state_of_charge_can_data;
            CANbus_Send(state_of_charge_msg, CAN_BLOCKING, CARCAN);
            printf("\r\nState of Charge: %d", SOC);
        }

        printf("\r\n"); 
        printf("\r\n"); 
        printf("\r\n"); 

        printf("\n\r=========== Testing BPS TRIP ===========");
        printf("\n\rNote: Test will end after this following message.\n\r"); 
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
        CANbus_Send(bps_trip_msg, CAN_BLOCKING, CARCAN);
        printf("BPS Trip was unsucessful");     // Prints message if unsucesssful
        
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);

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
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);

    while(1){};
}