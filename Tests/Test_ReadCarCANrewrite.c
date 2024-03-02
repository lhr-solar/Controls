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

/*** Test menu ***/
//#define TEST_RENODE      // Renode test does not work unless updatePrechargeContactor is added to this file
//#define TEST_HARDWARE_HV_ARRAY_ON
//#define TEST_HARDWARE_HV_PLUS_MINUS_ON
#define TEST_HARDWARE_HV_CONTACTORS_BOTH_ON
//#define TEST_HARDWARE_HV_CONTACTORS_BOTH_OFF

/*** How many msgs to send based on contactor status ***/
enum {
    SEND_ONE_MSG = 0, 
    SEND_UNTIL_ARRAY_ON,
    SEND_UNTIL_MOTOR_CONT_ON
};

/*** Constants ***/
#define SATURATION_THRESHOLD_TEST (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

/*** Tasks ***/
static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

#ifdef TEST_HARDWARE_HV_CONTACTORS_BOTH_OFF
static CANDATA_t HV_Disable_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b000}};
#endif

#ifdef TEST_HARDWARE_HV_ARRAY_ON
static CANDATA_t HV_Array_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b001}};
#endif

#ifdef TEST_HARDWARE_HV_CONTACTORS_BOTH_ON
static CANDATA_t HV_Enabled_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b111}};
#endif

#ifdef TEST_HARDWARE_HV_PLUS_MINUS_ON
static CANDATA_t HV_MC_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b110}};
#endif

#ifdef TEST_RENODE

/*** CAN Messages ***/
static CANDATA_t bps_trip_msg = {.ID=BPS_TRIP, .idx=0, .data={1}};
static CANDATA_t supp_voltage_msg = {.ID=SUPPLEMENTAL_VOLTAGE, .idx=0, .data={100}};
static CANDATA_t state_of_charge_msg = {.ID=STATE_OF_CHARGE, .idx=0, .data={0}};
static CANDATA_t HV_Array_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b001}};
static CANDATA_t HV_Disable_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b000}};
static CANDATA_t HV_MC_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b110}};
static CANDATA_t HV_Enabled_Msg = {.ID=BPS_CONTACTOR, .idx=0, .data={0b111}};
static uint8_t truthTableCounter = 0; 

static void infoDump(){
    
    printf("\n\r%d", truthTableCounter);
    truthTableCounter++;
    /*updatePrechargeContactors();*/ // To run testfile: uncomment and add the updatePrechargeContactors function in the ReadCarCAN.h file

    printf("\r\nArray Contactor          : %s", ((Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR) == ON) ? "ON" : "OFF")); 
    // printf("\r\nArray Ignition Status    : %s", ((ArrayIgnitionStatus_Get()) ? "ON" : "OFF"));
    // printf("\r\nCharge Message Saturation: %d", HVArrayMsgSaturation_Get());
    // printf("\r\nThreshold                : %s", ((HVArrayMsgSaturation_Get() >= 7.5) ? "Threshold reached" : "Threshold not reached"));
    // printf("\r\nCharge Enable            : %s", (ChargeEnable_Get() ? "TRUE" : "FALSE"));   

    printf("\r\nMotor Contactor          : %s", ((Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR) == ON) ? "ON" : "OFF"));
    // printf("\r\nMotor Ignition Status    : %s", ((MotorControllerIgnition_Get()) ? "ON" : "OFF"));
    // printf("\r\nCharge Message Saturation: %d", PlusMinusMsgSaturation_Get());
    // printf("\r\nThreshold                : %s", ((PlusMinusMsgSaturation_Get() >= 7.5) ? "Threshold reached" : "Threshold not reached"));
    // printf("\n\r");
    printf("\n\r");
}

static void turnIgnitionToMotorOn(){
    printf("\n\r=========== Turn Ignition to Motor ===========");
    Minions_Write(IGN_2, 0);                      // Ignition motor ON
    Minions_Write(IGN_1, 1);                      // Ignition array OFF
}

static void turnIgnitionToArrayOn(){
    printf("\n\r=========== Turn Ignition to Array ===========");
    Minions_Write(IGN_2, 1);                      // Ignition motor OFF
    Minions_Write(IGN_1, 0);                      // Ignition array ON
}

static void turnIgnitionOFF(){
    printf("\n\r=========== Turn Ignition to OFF ===========");
    Minions_Write(IGN_2, 1);                      // Ignition motor OFF
    Minions_Write(IGN_1, 1);                      // Ignition array OFF
}

static void turnIgnitionBoth(){
    printf("\n\r=========== Turn Ignition to Both ===========");
    Minions_Write(IGN_2, 0);                      // Ignition motor OFF
    Minions_Write(IGN_1, 0);                      // Ignition array OFF
}


static void sendArrayEnableMsg(uint8_t isIgnitionOn){
    printf("\n\r=========== Array Enable Msg Sent ===========");
    if(isIgnitionOn == 1){
        while(!Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR)){
            CANbus_Send(HV_Array_Msg, CAN_BLOCKING, CARCAN); 
            }   
     }else{
            CANbus_Send(HV_Array_Msg, CAN_BLOCKING, CARCAN);       
     }
}


static void sendMotorControllerEnableMsg(uint8_t isIgnitionOn){
    printf("\n\r=========== Motor Controller Enable Msg Sent ===========");
    if(isIgnitionOn == SEND_UNTIL_MOTOR_CONT_ON){
        while(Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR) != true){
            CANbus_Send(HV_MC_Msg, CAN_BLOCKING, CARCAN);      
        }
    }else{
            CANbus_Send(HV_MC_Msg, CAN_BLOCKING, CARCAN);     
    }
}


static void sendDisableMsg(){
    printf("\n\r=========== Disable Msg Sent ===========");
    CANbus_Send(HV_Disable_Msg, CAN_BLOCKING, CARCAN);      
}

static void sendEnableMsg(int isIgnitionOn){
    printf("\n\r=========== Enable Msg Sent ===========");

         if (isIgnitionOn == SEND_UNTIL_ARRAY_ON){
            while(Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR) == false){
                CANbus_Send(HV_Enabled_Msg, CAN_BLOCKING, CARCAN); 
            }    
        }
        if(isIgnitionOn == SEND_UNTIL_MOTOR_CONT_ON){
            while(Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR) == false){
                CANbus_Send(HV_Enabled_Msg, CAN_BLOCKING, CARCAN); 
                }     

            CANbus_Send(HV_Enabled_Msg, CAN_BLOCKING, CARCAN);     
        
    }
}

static void turnContactorOn(uint8_t isIgnitionOn){
    if(isIgnitionOn == SEND_UNTIL_ARRAY_ON){
        turnIgnitionToArrayOn();
        sendArrayEnableMsg(SEND_UNTIL_ARRAY_ON);
    }else if(isIgnitionOn == SEND_UNTIL_MOTOR_CONT_ON){
        turnIgnitionToMotorOn();
        sendMotorControllerEnableMsg(SEND_UNTIL_MOTOR_CONT_ON);
    }
}

#endif

void Task1(){
    OS_ERR err;

    CPU_Init();
    Minions_Init();
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
        (OS_PRIO)1,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);
    
    while(1){
        #ifdef TESTRENODE

            // Ignition is in OFF position

            // Case 0
            turnContactorOn(SEND_UNTIL_MOTOR_CONT_ON); // Turns ign to motor and sends enough messages to turn MC PBC on 
            turnIgnitionOFF();
            sendDisableMsg();
            infoDump();

            // Case 1
            turnContactorOn(SEND_UNTIL_ARRAY_ON);
            turnIgnitionOFF(); 
            sendArrayEnableMsg(SEND_ONE_MSG);
            infoDump();  

            // Case 2
            turnContactorOn(SEND_UNTIL_MOTOR_CONT_ON); 
            turnIgnitionOFF();
            sendMotorControllerEnableMsg(SEND_ONE_MSG);
            infoDump(); 

            // Case 3
            turnContactorOn(SEND_UNTIL_MOTOR_CONT_ON); 
            turnIgnitionOFF();
            /*updatePrechargeContactors();*/
            sendEnableMsg(SEND_ONE_MSG);
            infoDump();

            // Ignition is in ARRAY position
            // Case 4
            turnIgnitionToArrayOn();
            sendDisableMsg();
            infoDump();

            // Case 5
            turnIgnitionToArrayOn();
            sendArrayEnableMsg(SEND_UNTIL_ARRAY_ON);
            infoDump();

            // Case 6
            turnIgnitionToArrayOn();
            sendMotorControllerEnableMsg(SEND_UNTIL_ARRAY_ON);
            infoDump();

            // Case 7
            turnIgnitionToArrayOn();
            sendEnableMsg(SEND_UNTIL_ARRAY_ON); 
            infoDump();

            // Ignition is in MOTOR position
            // Case 8
            turnIgnitionToMotorOn();
            sendDisableMsg(); // turns off after one msg so do not need to keep sending until off
            infoDump();

            // Case 9
            turnIgnitionToMotorOn();
            sendArrayEnableMsg(SEND_UNTIL_ARRAY_ON);
            infoDump();

            // Case 10
            turnIgnitionToMotorOn();
            sendMotorControllerEnableMsg(SEND_UNTIL_MOTOR_CONT_ON);
            infoDump();

            // Case 11
            turnIgnitionToMotorOn();
            sendEnableMsg(SEND_UNTIL_ARRAY_ON); 
            sendEnableMsg(SEND_UNTIL_MOTOR_CONT_ON); 
            sendEnableMsg(SEND_UNTIL_ARRAY_ON); 
            infoDump();

            // Ignition is in IMPOSSIBLE array and motor position (both at the same time)
            turnIgnitionBoth();
            sendDisableMsg();
            infoDump();

            turnIgnitionBoth();
            // Test stops working past this point because array PBC never closes
            sendArrayEnableMsg(SEND_UNTIL_ARRAY_ON);
            infoDump();

            turnIgnitionBoth();
            sendMotorControllerEnableMsg(SEND_UNTIL_ARRAY_ON);
            infoDump();

            turnIgnitionBoth();
            sendEnableMsg(SEND_UNTIL_ARRAY_ON); 
            infoDump();



            // Test case for supply voltage
            printf("\n\r");
            printf("\n\r=========== Testing: Supply Voltage ===========");
            CANbus_Send(supp_voltage_msg, CAN_BLOCKING, CARCAN); // Supply Voltage message
            //printf("Supply Voltage: %ld", SBPV_Get());
                
            // Test case for supply voltage
            printf("\n\r");
            printf("\n\r=========== Testing: State of Charge ===========");
            *(uint64_t*)(&state_of_charge_msg.data) = 42000000;
            CANbus_Send(state_of_charge_msg, CAN_BLOCKING, CARCAN); // State of Charge message
            //printf("State of Charge: %d",SOC_Get());
                

            printf("\n\n\rtest done, nothing should print after bps trip\r");

            // Test case for BPS Trip
            printf("\n\r");
            printf("\n\r=========== Testing: BPS Trip ===========");
            CANbus_Send(bps_trip_msg, CAN_BLOCKING, CARCAN); // BPS Trip
            infoDump();
        
            #endif 
            OS_ERR err;
            #ifdef TEST_HARDWARE_HV_ARRAY_ON
                while(1){
                    CANbus_Send(HV_Array_Msg, CAN_BLOCKING, CARCAN); // HV Array messages
                    printf("\r\nArray PBC: %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\nMC PBC:    %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\n");
                    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
                }
            #endif

            #ifdef TEST_HARDWARE_HV_PLUS_MINUS_ON
                while(1){
                    CANbus_Send(HV_MC_Msg, CAN_BLOCKING, CARCAN); // HV Motor Controller messages
                    printf("\r\nArray PBC: %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\nMC PBC:    %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\n");
                    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
                }
            #endif

            #ifdef TEST_HARDWARE_HV_CONTACTORS_BOTH_ON
                while(1){
                    CANbus_Send(HV_Enabled_Msg, CAN_BLOCKING, CARCAN); // HV Enable messages
                    printf("\r\nArray PBC: %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\nMC PBC:    %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\n");
                    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
                }
            #endif

            #ifdef TEST_HARDWARE_HV_CONTACTORS_BOTH_OFF
                while(1){
                    CANbus_Send(HV_Disable_Msg, CAN_BLOCKING, CARCAN); // Disable messages
                    printf("\r\nArray PBC: %d", Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\nMC PBC:    %d", Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR));
                    printf("\r\n");
                    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
                }
            #endif
            
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
    assertOSError(err);

    OSStart(&err);
    assertOSError(err);

    while(1){};
}