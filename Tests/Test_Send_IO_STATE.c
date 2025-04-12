/**
 * This file tests if we can send IO_STATE to the contactor board
 * Goal: verify that CAN messages we send can be properly received by the contactor board
 */


 #include "Tasks.h"
 #include "CANbus.h"
 #include "BSP_UART.h"
 #include "CANConfig.h"
 #include "Minions.h"
 #include "Pedals.h"
 #include "UpdateDisplay.h"
 
 static OS_TCB Task1_TCB;
 static CPU_STK Task1_Stk[128];
 
 #define STACK_SIZE 128 
 #define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))


/********** Test options ************/
#define FILTER_ON
#define READ_INPUTS
#define PRINT_INPUTS
 
 static void assert(bool);
 
 CANDATA_t dataBuf, resultBuf;
 uint64_t data = 0xdeadbeef12345678;
 
 /*
  * NOTE: This test must be run with car CAN in loopback mode
  * TODO: automate this, either with arguments to BSP or #define
  */
 
 void Task1(void *p_arg) {
    (void) p_arg;
    OS_ERR err;
    // DisplayError_t error;

    // Initalization
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    Minions_Init();
    // error = Display_Init();
    // UpdateDisplay_Init();
    
    // OS_ERR e;

    // OSTaskCreate(
    //     (OS_TCB *)&UpdateDisplay_TCB,
    //     (CPU_CHAR *)"UpdateDisplay_TCB",
    //     (OS_TASK_PTR)Task_UpdateDisplay,
    //     (void *)NULL,
    //     (OS_PRIO)13,
    //     (CPU_STK *)UpdateDisplay_Stk,
    //     (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
    //     (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
    //     (OS_MSG_QTY)0,
    //     (OS_TICK)NULL,
    //     (void *)NULL,
    //     (OS_OPT)(OS_OPT_TASK_STK_CLR),
    //     (OS_ERR *)&e);
    // assertOSError(e);

    // Decide if we should filter
    #ifdef FILTER_ON
    CANbus_Init(CARCAN, carCANFilterList, NUM_CARCAN_FILTERS);
    #else
    CANbus_Init(CARCAN, NULL, 0);
    #endif

    // uint8_t last_pins = 0;
    uint8_t pins = 0;

    #ifdef READ_INPUTS
    int8_t accel = 0, brake = 0;
    #endif

    while (1) {
        
        #ifdef READ_INPUTS
        data = 0;

        // last_pins = pins;

        pins = 0;
    
        // Precision 32 bits Data: [23: 16] : Switch Bitmap (from bit 7 to 0; BRAKELIGHT, CRUZ_ST, CRUZ_EN, REV_SW, FOR_SW, REGEN_SW, IGN_2_Motor, IGN_1_Array) [15:8] : Brake Pedal [7: 0] : Accel Pedal Frequency: 250ms	
        for (int i = 0; i < NUM_PINS; i++) {
            pins = pins | (Minions_Read(i) << i);
        }
        accel = Pedals_Read(ACCELERATOR);
        brake = Pedals_Read(BRAKE);

        data = data | ((pins << 16) + (brake << 8) + accel);
        #endif

         // Optionally print dashboard/pedal inputs
        #ifdef PRINT_INPUTS
        printf("\n\rPins (7-0): ");
        for (int i = NUM_PINS - 1; i > -1; i--) {
            printf("%d ", Minions_Read(i));
        }
        printf("\n\r----- Pin interpretation-----");
        printf("\n\r BRAKELIGHT: %d", Minions_Read(BRAKELIGHT));
        printf("\n\r CRUZ_ST: %d", Minions_Read(CRUZ_ST));
        printf("\n\r CRUZ_EN: %d", Minions_Read(CRUZ_EN));
        printf("\n\r REV_SW: %d", Minions_Read(REV_SW));
        printf("\n\r FOR_SW: %d", Minions_Read(FOR_SW));
        printf("\n\r BPS_HAZ: %d", Minions_Read(BPS_HAZ));
        printf("\n\r IGN_2: %d", Minions_Read(IGN_2));
        printf("\n\r IGN_1: %d", Minions_Read(IGN_1));

        printf("\n\r Accel: %d", Pedals_Read(ACCELERATOR));
        printf("\n\r Brake: %d", Pedals_Read(BRAKE));
        #endif 


        // Copy data and send CAN message
        dataBuf.ID = CONTACTOR_SENSE;
        memcpy(&dataBuf.data, &data, sizeof data); // 0xdeadbeef12345678 or inputs;
        assert(CANbus_Send(dataBuf, true, CARCAN) == SUCCESS);

        // Print CAN message
        printf("\n\rSend msg ID: %x, Data: 0x", dataBuf.ID);
        for (int i = 7; i >= 0; i--) {
            printf("%x", dataBuf.data[i]);
        }
        
        // Check if we've received a CAN message; print if true
        if (CANbus_Read(&resultBuf, CAN_NON_BLOCKING, CARCAN)) {
            printf("\n\r         Read msg ID: %x, Data: 0x", resultBuf.ID);
            for (int i = 7; i >= 0; i--) {
                printf("%x", resultBuf.data[i]);
            }
        } 

        // #ifdef DISPLAY_INPUTS
        // if (last_pins != pins) {
        //    UpdateDisplay_SetAccel();
        //    UpdateDisplay_SetBrake();
        //    UpdateDisplay_SetArray(Minions_read())
        // }

        // printf("\n\r CRUZ_ST: %d", Minions_Read(6));
        // printf("\n\r CRUZ_EN: %d", Minions_Read(5));
        // printf("\n\r REV_SW: %d", Minions_Read(4));
        // printf("\n\r FOR_SW: %d", Minions_Read(3));
        // printf("\n\r BPS_HAZ: %d", Minions_Read(2));
        // printf("\n\r IGN_2: %d", Minions_Read(1));
        // printf("\n\r IGN_1: %d", Minions_Read(0));

        // printf("\n\r Accel: %d", Pedals_Read(ACCELERATOR));
        // printf("\n\r Brake: %d", Pedals_Read(BRAKE));



        // Loop every 250ms
        OSTimeDlyHMSM(0, 0, 0, 250, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
 }

 
 int main(void){ //initialize things and spawn task
     OS_ERR err;
     OSInit(&err);
     if(err != OS_ERR_NONE){
         printf("OS error code %d\n\r",err);
     }
 
     BSP_UART_Init(USB);
     printf("\n\rBSP Initialized");
 
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
 
     
     if (err != OS_ERR_NONE) {
         printf("Task1 error code %d\n\r", err);
     }
     OSStart(&err);
     if (err != OS_ERR_NONE) {
         printf("OS error code %d\n\r", err);
     }
     return 0;
 
 }
 
 static void assert(bool cond) {
     if (!cond) __asm("bkpt");
 }
 