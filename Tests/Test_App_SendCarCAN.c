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

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];
static OS_TCB TaskReadCAN_TCB;
static CPU_STK TaskReadCANStk[DEFAULT_STACK_SIZE];

// Reads CarCAN and prints what we receive
void Task_ReadCAN(void *arg)
{
    OS_ERR err;
	CANDATA_t dataBuf = {0};
    uint32_t ts;


    while (1) {
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);

		if (status == SUCCESS){
            ts = (OSTimeGet(&err) / OS_CFG_TICK_RATE_HZ);

            printf("\n\r********* Received CANbus message **********");
            printf("\n\rAt: %5ld ms", ts);
            printf("\n\r Received %x %d", dataBuf.data[0], dataBuf.data[4]);

			switch(dataBuf.ID){
				case MOTOR_STATUS:{
                    // Motor status error flags is in bytes 4-5
                    printf("\n\r Received MOTOR_STATUS message of %d", *((uint16_t*)(&dataBuf.data[4])));
					break;
				}
				
				case VELOCITY:{
                    // Motor RPM is in bytes 0-3, Car Velocity (in m/s) is in bytes 4-7
                    printf("\n\r Received VELOCITY message of ");
                    printf("\n\rMotor RPM: %f\n\rMotor Velocity: %f", *((float*)(&dataBuf.data[0])), *((float*)(&dataBuf.data[4])));
                    break;
				}

                case IO_STATE: { 
                    printf("\n\rReceived IO_STATE message of %x", dataBuf.data[0]);
                    printf("\n\rAccelerator: %d", *((uint8_t*)(&dataBuf.data[0])));
                    printf("\n\rBrake: %x", *((uint8_t*)(&dataBuf.data[1])));
                    printf("\n\rPins: %x", dataBuf.data[2]);
                    printf("\n\rContactors: %d", *((uint8_t*)(&dataBuf.data[3])));
                    break;

                }

                case CONTROL_MODE: {
                    printf("\n\rReceived CONTROL_MODE message of %d", *((uint8_t*)(&dataBuf.data[0])));
                    break;
                }

				default:{
                    printf("\n\rReceived an uncategorized message ID type of %x", dataBuf.ID);
					break; //for cases not handled currently
				}
            }
        } else {
            // If status != no error
            printf("\n\rCANbus_Read error of %x", status);
        }  

        printf("\n\r**********************************");
    }


}

void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    CANbus_Init(CARCAN, (CANId_t*)carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    Minions_Init();
    //UpdateDisplay_Init(); // Do we need this? Are we using the display?

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    // do we need regenEnable = ON;?

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
    assertOSError(OS_MAIN_LOC, err);

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
    assertOSError(OS_MAIN_LOC, err);

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
    assertOSError(OS_MAIN_LOC, err);

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
    assertOSError(OS_MAIN_LOC, err);

   
    while (1) {
        // Print Car State (done in SendTritium)
        // Print IO State (need to do this)
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

        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
    }   

}


int main(void) 
{
    OS_ERR err;
    OSInit(&err);

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
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);
}