/**
 * This test file for SendCarCAN checks if messages added to the queue are sent on CarCAN.
 * It does this by starting the SendTritium, SendCarCAN, and ReadTritium tasks,
 * which place messages into the SendCarCAN_Q, and then printing messages read on CarCAN via UART.
 * It also prints the IO_State and queue status to see if the messages are accurate and the queue has space.
 * 
 * This test is intended to be run on hardware with CarCAN in LoopBack mode.
 * It is also intended to be tested using the motor controller on to read motor messages.
 * Alternatively, you can just use it to spin up tasks and then check output using a logic analyzer.
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

#define NUM_MOTOR_MSGS 15 // Messages received from the motor controller
#define MOTOR_MSG_BASE_ADDRESS 0x240


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];
static OS_TCB TaskReadCAN_TCB;
static CPU_STK TaskReadCANStk[DEFAULT_STACK_SIZE];

static uint32_t msgCount[20];

// Reads CarCAN and prints what we receive on UART
void Task_ReadCAN(void *arg)
{
    //OS_ERR err;
	CANDATA_t dataBuf = {0};
    //uint32_t ts;


    while (1) {
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);

		if (status == SUCCESS){
            switch(dataBuf.ID) {
                case IO_STATE: {
                    msgCount[0]++;
                    printf("\n\rReceived IO_STATE message of %x", dataBuf.data[0]);
                    printf("\n\rAccelerator: %d", *((uint8_t*)(&dataBuf.data[0])));
                    printf("\n\rBrake: %x", *((uint8_t*)(&dataBuf.data[1])));
                    printf("\n\rPins: %x", dataBuf.data[2]);
                    printf("\n\rContactors: %d", *((uint8_t*)(&dataBuf.data[3])));
                    break;
                }

                case CONTROL_MODE: {
                    msgCount[1]++;
                    break;
                }

                default: {
                    uint8_t msgIdx = dataBuf.ID - MOTOR_MSG_BASE_ADDRESS;

                    if (msgIdx >= 0 && msgIdx < NUM_MOTOR_MSGS) {  // Check if the message is from the motor controller
                        msgCount[msgIdx + 2]++; // Reset counter and continue sending
                    } else {
                        msgCount[20]++;
                    }
                     
                }

            }
            
            // ts = (OSTimeGet(&err) / OS_CFG_TICK_RATE_HZ);

            // printf("\n\r********* Received CANbus message **********");
            // printf("\n\rAt: %5ld ms", ts);
            // printf("\n\r Received %x %d", dataBuf.data[0], dataBuf.data[4]);

			// switch(dataBuf.ID){

            //     case IO_STATE: { 
            //         printf("\n\rReceived IO_STATE message of %x", dataBuf.data[0]);
            //         printf("\n\rAccelerator: %d", *((uint8_t*)(&dataBuf.data[0])));
            //         printf("\n\rBrake: %x", *((uint8_t*)(&dataBuf.data[1])));
            //         printf("\n\rPins: %x", dataBuf.data[2]);
            //         printf("\n\rContactors: %d", *((uint8_t*)(&dataBuf.data[3])));
            //         break;

            //     }

            //     case CONTROL_MODE: {
            //         printf("\n\rReceived CONTROL_MODE message of %d", *((uint8_t*)(&dataBuf.data[0])));
            //         break;
            //     }

    

			// 	default:{ // Other messages will only be received if CARCAN messages aren't filtered.
            //         printf("\n\rReceived an uncategorized message ID type of %x", dataBuf.ID);
			// 		break; //for cases not handled currently
			// 	}
        } else {
            // CANbus read is unsuccessful
            printf("\n\rCANbus_Read error of %x", status);
        }  

        //printf("\n\r**********************************");
    }


}



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
        (CPU_CHAR*)"Test_ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCAN,
        (void*)NULL,
        (OS_PRIO)13, // Lower prio than other running tasks
        (CPU_STK*)TaskReadCANStk,
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

   
    while (1) {
        // SendTritium task will print Car State

        // Print IO State
        printf("\n\r---- IO State ----");
        printf("\n\rAccelerator: %d", Pedals_Read(ACCELERATOR));
        printf("\n\rBrake: %d", Pedals_Read(BRAKE));
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
        // Print how full CAN Queue is 
        printf("\n\r---- Queue data ----");
        print_SendCarCAN_Q();

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
    CANbus_Init(CARCAN, NULL, NUM_CARCAN_FILTERS); // Set filter list to (CANId_t*)carCANFilterList to receive filtered messages
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    SendCarCAN_Init();
    Minions_Init();
    Display_Init();
    TaskSwHook_Init();

    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1, 
        (void *)NULL,
        (OS_PRIO)13, //
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0, 
        (OS_TICK)NULL,
        (void *)NULL, //
        (OS_OPT)(OS_OPT_TASK_STK_CLR), 
        (OS_ERR *)&err
    );
    assertOSError(err);

    OSStart(&err);
    assertOSError(err);
}