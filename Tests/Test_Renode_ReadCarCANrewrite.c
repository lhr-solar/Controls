#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "ReadCarCAN.h"
#include "Contactors.h"
#include "Display.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"

static OS_TCB Task1_TCB;
#define STACK_SIZE 128
static CPU_STK Task1_Stk[STACK_SIZE];

CANDATA_t bps_trip_msg = {
    BPS_TRIP, 		
	0, 		
	0,
};

CANDATA_t charge_enable_msg = {
    CHARGE_ENABLE, 		
	0, 		
	0,
};

CANDATA_t supp_voltage_msg = {
    SUPPLEMENTAL_VOLTAGE, 		
	0, 		
	0,
};

CANDATA_t state_of_charge_msg = {
    STATE_OF_CHARGE, 		
	0, 		
	0,
};

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

void Task1(){
    OS_ERR err;
    CPU_TS ts;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    Contactors_Init();
    Contactors_Enable(ARRAY_CONTACTOR);
    Contactors_Enable(MOTOR_CONTACTOR);
    Contactors_Enable(ARRAY_PRECHARGE);
    CANbus_Init(CARCAN, &carCANFilterList, CARCAN_FILTER_SIZE);
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
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    while(1){
        printf("\n\r=========== Testing Precharge ===========");
        for(int i = 0; i < 10; i++){
            CANbus_Send(charge_enable_msg, true, CARCAN);
            printf("\r\nCharge Enable            :   %d", chargeEnable_Get());
            printf("\r\nCharge Enable should be  :   1");
            printf("\r\nCharge Message Saturation:   %f", chargeMsgSaturation);
            printf("\r\nCharge Message Threshold :   %f", 7.5);
        }

        printf("\n\r=========== Testing Disable ===========");
        for(int i = 0; i < 10; i++){
            CANbus_Send(charge_enable_msg, true, CARCAN);
            printf("\r\nCharge Enable            :   %d", chargeEnable_Get());
            printf("\r\nCharge Enable should be  :   0");
            printf("\r\nCharge Message Saturation:   %f", chargeMsgSaturation);
            printf("\r\nCharge Message Threshold :   %f", 7.5);
            // should print fault state error for charge disable
        }

        printf("\n\r=========== Testing Supp Voltage ===========");
        CANbus_Send(supp_voltage_msg, true, CARCAN);
        print("\r\nSupplemental Voltage: %f", SBPV);

        printf("\n\r=========== Testing State of Charge ===========");
        CANbus_Send(state_of_charge_msg, true, CARCAN);
        print("\r\nState of Charge: %f", SOC);

        printf("\n\r=========== Testing BPS TRIP ===========");
        CANbus_Send(bps_trip_msg, true, CARCAN);
        // should print the fault state BPS trip message
        
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
        (CPU_STK_SIZE)STACK_SIZE/10,
        (CPU_STK_SIZE)STACK_SIZE,
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