#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

#include "SendTritium_MVP.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

// velocity options based on relevant thresholds
static typedef enum {
    VEL_ABOVE_GEARSWITCH_THRESH,
    VEL_WITHIN_GEARSWITCH_THRESH,
    VEL_ABOVE_NEG_GEARSWITCH_THRESH,
} velocity_opts_t;

// // brake pressed(100) or break unpressed(0)
// typedef enum {
//     BRAKE_UNPRESSED,
//     BRAKE_PRESSED,
// } brake_opts_t;

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

/**
 * ======= Testing Utilities ==========
 * Wrappers to set velocity & brake according to enum parameters
*/
static void set_velocityObserved_wrap(velocity_opts_t velObservedEnum) {
    switch(velObservedEnum){
        case VEL_ABOVE_GEARSWITCH_THRESH:
            set_velocityObserved(MAX_GEARSWITCH_VELOCITY + 1);
            break;
        case VEL_WITHIN_GEARSWITCH_THRESH:
            set_velocityObserved(0);
            break;
        case VEL_ABOVE_NEG_GEARSWITCH_THRESH:
            set_velocityObserved(-MAX_GEARSWITCH_VELOCITY - 1);
            break;            
    }
}

// static void set_brake_wrap(brake_opts_t brakeEnum) {
//     if (brakeEnum == BRAKE_UNPRESSED)
//         set_brakePedalPercent(0);
//     else if (brakeEnum == BRAKE_PRESSED)
//         set_brakePedalPercent(100);    
// }

/**
 * ======= State Setters ==========
 * States:
 * Forward Drive, Park State, Reverse Drive
*/
void goToForwardDrive(velocity_opts_t velObservedEnum){
    // Set forward drive state & gear
    set_state(FORWARD_DRIVE);
    set_gear(FORWARD_GEAR);

    // Set velocityObserved
    set_velocityObserved_wrap(velObservedEnum);
}

void goToParkState(velocity_opts_t velObservedEnum){
    // Set park state & gear
    set_state(PARK_STATE);
    set_gear(PARK_GEAR);

    // Set velocityObserved
    set_velocityObserved_wrap(velObservedEnum);
}

void goToReverseDrive(velocity_opts_t velObservedEnum){
    // Set reverse drive state & gear
    set_state(REVERSE_DRIVE);
    set_gear(REVERSE_GEAR);

    // Set velocityObserved
    set_velocityObserved_wrap(velObservedEnum);
}



void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    CANbus_Init(MOTORCAN, NULL, 0);
    Minions_Init();
    UpdateDisplay_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    set_regenEnable(ON);

    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*) NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    //assertOSError(err);
    /**
     * ======= Forward Drive ==========
     * State Transitions: 
     * Park, Reverse
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Park
    printf("\n\rTesting: Forward Drive -> Park\n\r");
    goToForwardDrive(VEL_ABOVE_GEARSWITCH_THRESH);
    set_gear(PARK_GEAR);
    stateBuffer();
    while(get_state().name != PARK_STATE){}

    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Forward Drive -> Reverse Drive\n\r");
    goToForwardDrive(VEL_WITHIN_GEARSWITCH_THRESH); // Velocity w/in Gearswitch Threshold
    set_gear(REVERSE_GEAR);
    stateBuffer();
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}
    goToForwardDrive(VEL_ABOVE_GEARSWITCH_THRESH);  // Velocity > Gearswitch Threshold
    set_gear(REVERSE_GEAR);
    stateBuffer();
    stateBuffer();
    while(get_state().name != PARK_STATE){}


    /**
     * ======= Park State ==========
     * State Transitions: 
     * Forward Drive, Reverse Drive
    */

    printf("\n\r============ Testing Park State ============\n\r");

    // Park to Forward Drive
    printf("\n\rTesting: Park -> Forward Drive\n\r");
    goToParkState(VEL_ABOVE_GEARSWITCH_THRESH);    // Velocity > Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}
    goToParkState(VEL_WITHIN_GEARSWITCH_THRESH);    // Velocity w/in Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}
    goToParkState(VEL_ABOVE_NEG_GEARSWITCH_THRESH); // Velocity < -Gearswitch Threshold
    set_gear(FORWARD_GEAR);
    stateBuffer();
    while(get_state().name != PARK_STATE){}

    // Park to Reverse Drive
    printf("\n\rTesting: Park -> Reverse Drive\n\r");
    goToParkState(VEL_ABOVE_GEARSWITCH_THRESH);    // Velocity > Gearswitch Threshold  
    set_gear(REVERSE_GEAR);
    stateBuffer();
    while(get_state().name != PARK_STATE){}
    goToParkState(VEL_WITHIN_GEARSWITCH_THRESH);    // Velocity w/in Gearswitch Threshold  
    set_gear(REVERSE_GEAR);
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}
    goToParkState(VEL_ABOVE_NEG_GEARSWITCH_THRESH); // Velocity < -Gearswitch Threshold
    set_gear(REVERSE_GEAR);
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

    /**
     * ======= Reverse Drive ==========
     * State Transitions: 
     * Park, Forward Drive
    */

    printf("\n\r============ Testing Reverse Drive State ============\n\r");
    
    // Reverse Drive to Brake
    printf("\n\rTesting: Reverse Drive -> Park\n\r");
    goToReverseDrive(VEL_ABOVE_NEG_GEARSWITCH_THRESH);
    set_gear(PARK_GEAR)
    stateBuffer();
    while(get_state().name != PARK_STATE){}

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    goToReverseDrive(VEL_WITHIN_GEARSWITCH_THRESH);     // Velocity w/in Gearswitch Threshold
    set_gear(FORWARD_GEAR);
    stateBuffer();
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}
    goToReverseDrive(VEL_ABOVE_NEG_GEARSWITCH_THRESH);  // Velocity < -Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateBuffer();
    stateBuffer();
    while(get_state().name != PARK_STATE){}


    OS_TaskSuspend(&SendTritium_TCB, &err);
    assertOSError(err);
    while (1){
        printf("\n\r\n\rSUCCESS! ALL TESTS PASSED\n\r\n\r");
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);
    OSSemCreate( // create fault sem4
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err);
    assertOSError(err);

    OSTaskCreate( // create fault task
        (OS_TCB *)&FaultState_TCB,
        (CPU_CHAR *)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void *)NULL,
        (OS_PRIO)1,
        (CPU_STK *)FaultState_Stk,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
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
        (OS_ERR *)&err);
    assertOSError(err);

    OSStart(&err);
}