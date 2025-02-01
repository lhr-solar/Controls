/**
 * This test file for SendCarCAN checks if messages added to the queue are sent on CarCAN.
 * It does this by starting the SendTritium, SendCarCAN, and ReadTritium tasks,
 * which place messages into the SendCarCAN_Q, and then counting the CarCAN messages it receives in msgReadCount.
 * It also prints the IO State and, if the SendTritium macro SENDTRITIUM_PRINT_MES is defined, the control mode
 * for comparison to see if the data received on CarCAN is accurate.
 * 
 * This test is intended to be run on hardware with CarCAN in LoopBack mode and the motor controller connected.
 * The idea is to display the struct msgReadCount using a debugger view what messages we've received.
 * Alternatively, you can just use it to spin up tasks and then check output using a logic analyzer.
 * 
 * If TEST_SOFTWARE is defined prior to compilation, then a fake "motor controller" task
 * will also be created to send us motor messages.
 * In this case, MotorCAN should also be in LoopBack mode.
*/

#include "Tasks.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "os_cfg_app.h"
#include "SendCarCAN.h"
#include "Minions.h"
#include "CANConfig.h"
#include "SendTritium.h"
#include "Contactors.h"
#include "Pedals.h"
#include "Display.h"
#include "UpdateDisplay.h"

#define TEST_SOFTWARE 

#define NUM_MOTOR_MSGS 15 // Messages we forward from the motor controller
#define NUM_NONMOTOR_MSGS 2 // We also send IO_STATE and CONTROL_MODE

// indexes in msgArray for tracking info 
#define IO_STATE_IDX 0 
#define CONTROL_MODE_IDX 1

#define MOTOR_MSG_BASE_ADDRESS 0x240 // Offset to index into msgCount array
// Test data stored for an individual message type
typedef struct {
    CANDATA_t lastMsg;
    uint16_t numReceived;
} msgInfo;

// Struct containing all test file message data
typedef struct {
    uint32_t lastReceiveTime_ms;
    uint16_t lastMsgReceived;
    msgInfo msgArray[NUM_MOTOR_MSGS + NUM_NONMOTOR_MSGS + 1]; // Message types we send plus an extra space 
    uint8_t SpaceLeftInQ;
} CANInfo;


static OS_TCB Task1_TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];
#ifdef TEST_SOFTWARE
static OS_TCB TaskMC_TCB;
static CPU_STK TaskMC_Stk[DEFAULT_STACK_SIZE];
#endif
static OS_TCB TaskReadCAN_TCB;
static CPU_STK TaskReadCAN_Stk[DEFAULT_STACK_SIZE];

static CANInfo msgReadCount = {0};

// Reads CarCAN and saves message statistics in the msgReadCount struct
void Task_ReadCAN(void *arg)
{
    OS_ERR err;
	CANDATA_t dataBuf = {0};


    while (1) {
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);

		if (status == SUCCESS){
            switch(dataBuf.ID) {
                case IO_STATE: {
                    msgReadCount.msgArray[IO_STATE_IDX].numReceived++;
                    msgReadCount.msgArray[IO_STATE_IDX].lastMsg = dataBuf;
                    break;
                }

                case CONTROL_MODE: {
                    msgReadCount.msgArray[CONTROL_MODE_IDX].numReceived++;
                    memcpy((void *)&msgReadCount.msgArray[CONTROL_MODE_IDX].lastMsg, &dataBuf, sizeof dataBuf);
                    break;
                }

                default: {
                    uint8_t msgIdx = dataBuf.ID - MOTOR_MSG_BASE_ADDRESS;

                    if (msgIdx >= 0 && msgIdx < NUM_MOTOR_MSGS) {  // Check if the message is from the motor controller
                        msgReadCount.msgArray[msgIdx + NUM_NONMOTOR_MSGS].numReceived++; // If so, increment based on the ID
                        memcpy((void *)&msgReadCount.msgArray[msgIdx + NUM_NONMOTOR_MSGS].lastMsg, &dataBuf, sizeof dataBuf);

                    } else { // Place messages not otherwise accounted for into the last index of the array
                        msgReadCount.msgArray[NUM_MOTOR_MSGS + NUM_NONMOTOR_MSGS].numReceived++;
                        memcpy((void *)(&msgReadCount.msgArray[NUM_MOTOR_MSGS + NUM_NONMOTOR_MSGS].lastMsg), &dataBuf, sizeof dataBuf);
                        
                    }
                     
                }

            }
            
            // Other CAN stats
            msgReadCount.lastReceiveTime_ms = (OSTimeGet(&err) / OS_CFG_TICK_RATE_HZ);
            msgReadCount.lastMsgReceived = dataBuf.ID;
            msgReadCount.SpaceLeftInQ = get_SendCarCAN_Q_Space();

        } else {
            // CANbus read is unsuccessful
            printf("\n\rCANbus_Read error of %x", status);
        }  
    }


}

#ifdef TEST_SOFTWARE
// Create a fake motor controller to send messages to the controls system via MotorCAN
void Task_MC(void *arg) {
    OS_ERR err;
    uint8_t loopCount = 0;
    CANDATA_t motorMsg = {
        .ID=VELOCITY,
        .idx=0,
        .data={0xD, 0xE, 0xA, 0xD, 0x0, 0x0, 0xE, 0xF}, // Bytes 4-5 store error flags and must be empty
    };
    
    while(1) {
        if (++loopCount == 5) { // Only send these messages every 1000ms
            loopCount = 0;
            motorMsg.ID=TEMPERATURE;
            CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
            motorMsg.ID=ODOMETER_AMPHOURS;
            CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        }
        // Send these messages every 200ms
        motorMsg.ID=MOTOR_STATUS;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        motorMsg.ID=MC_BUS;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        motorMsg.ID=MC_PHASE_CURRENT;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        motorMsg.ID=VOLTAGE_VEC;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        motorMsg.ID=CURRENT_VEC;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        motorMsg.ID=BACKEMF;
        CANbus_Send(motorMsg, CAN_NON_BLOCKING, MOTORCAN);
        OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &err);
    }
    
}
#endif


// Initialize other tasks and print IO state
void Task1(void *arg)
{
    OS_ERR err;

    UpdateDisplay_Init();

  // Initialize SendCarCAN
    OSTaskCreate(
        (OS_TCB*)&SendCarCAN_TCB,
        (CPU_CHAR*)"SendCarCAN",
        (OS_TASK_PTR)Task_SendCarCAN,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_CAR_CAN_PRIO,
        (CPU_STK*)SendCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    // Initialize SendTritium
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    // Initialize ReadTritium
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    // Create task to read CANbus
      OSTaskCreate(
        (OS_TCB*)&TaskReadCAN_TCB,
        (CPU_CHAR*)"Task_ReadCAN",
        (OS_TASK_PTR)Task_ReadCAN,
        (void*)NULL,
        (OS_PRIO)13, // Lower prio than other running tasks
        (CPU_STK*)TaskReadCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    // Initialize UpdateDisplay
    OSTaskCreate(
        (OS_TCB*)&UpdateDisplay_TCB,
        (CPU_CHAR*)"UpdateDisplay",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void*)NULL,
        (OS_PRIO)TASK_UPDATE_DISPLAY_PRIO,
        (CPU_STK*)UpdateDisplay_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_UPDATE_DISPLAY_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    #ifdef TEST_SOFTWARE
    // Create a task to simulate the motor controller
    OSTaskCreate(
        (OS_TCB*)&TaskMC_TCB,
        (CPU_CHAR*)"Task_MC",
        (OS_TASK_PTR)&Task_MC,
        (void*)NULL,
        (OS_PRIO)2, // The motor controller is "external" and shouldn't be delayed by other tasks
        (CPU_STK*)TaskMC_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);
    #endif

   
    while (1) {
        // (SendTritium task will print Control Mode for us)

        // Print IO State
        printf("\n\r---- IO State ----");
        printf("\n\rAccelerator: %u", Pedals_Read(ACCELERATOR));
        printf("\n\rBrake: %u", Pedals_Read(BRAKE));
        uint8_t pins = 0;
        for(pin_t pin = 0; pin < NUM_PINS; pin++){
            bool pinState = Minions_Read(pin);
            pins |= pinState << pin;
        }
        printf("\n\rMinions: %x", pins);
        uint8_t contactors = 0;
        for(contactor_t contactor = 0; contactor < NUM_CONTACTORS; contactor++){
            bool contactorState = (Contactors_Get(contactor) == ON) ? true : false;
            contactors |= contactorState << contactor;
        }
        printf("\n\rContactors: %x", contactors);
        OSTimeDlyHMSM(0, 0, 0, 10 * FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
    }   

}


int main(void) 
{
    OS_ERR err;
    OSInit(&err);

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    CANbus_Init(CARCAN, NULL, NUM_CARCAN_FILTERS);      // CarCAN filter list is normally (CANId_t*)carCANFilterList
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);  // but for testing, we'd like to receive all messages
    SendCarCAN_Init();
    Minions_Init();
    Display_Init();
    TaskSwHook_Init();

    OSTaskCreate(
        (OS_TCB *)&Task1_TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1, 
        (void *)NULL,
        (OS_PRIO)13, 
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0, 
        (OS_TICK)NULL,
        (void *)NULL, 
        (OS_OPT)(OS_OPT_TASK_STK_CLR), 
        (OS_ERR *)&err
    );
    assertOSError(err);

    OSStart(&err);
    assertOSError(err);
}