// NOTE: To full test, initiate the car in a non-neutral gear
// This ensures that the forced reset to neutral before the actual
// gear begins to be read occurs.

#include "Minions.h"
#include "Pedals.h"
#include "Dashboard.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "BSP_UART.h"
#include "Tasks.h"
#include "SendTritium.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void timeBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 150, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

void printFullState() {
    printf("-------------------\n\r");
    char gearName[20];
    switch(get_gear()) {
        case DASH_FWD:
            strcpy(gearName, "FORWARD: \n\r");
            break;
        case DASH_NEU:
            strcpy(gearName, "NEUTRAL: \n\r");
            break;
        case DASH_REV:
            strcpy(gearName, "REVERSE: \n\r");
            break;
        default:
            strcpy(gearName, "UNKOWN: \n\r");
            break;    
    }
    printf("brakePedalPercent: %u\n\r", get_brakePedalPercent());
    printf("accelPedalPercent: %u\n\r", get_accelPedalPercent());
    print_float("currentSetpoint: \n\r", get_currentSetpoint());
    print_float("velocitySetpoint: \n\r", get_velocitySetpoint());
    printf("----------------------------\n\r");
}


void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(USB);
    Pedals_Init();
    CANbus_Init(MOTORCAN, NULL, 0);
    Minions_Init();
    dashboardInit();
    UpdateDisplay_Init();
    BPSMotorFlags_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    // set_regenEnable(ON);

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
    assertOSError(err);

    // Motor & BPS status event flag group should've already been initialized to BPS_CHECKED, 
    // BPS_SAFE, & MOTOR_SAFE_TO_RUN cleared, so nothing should happen yet

    // Testing MOTOR_SAFE_TO_RUN
    timeBuffer();
    printf("\n\rTesting: Wait on MOTOR_SAFE_TO_RUN signal before running motor\n\r");
    printf("NOTE: We will first manually perform the forced driver reset to\n\r");
    printf("neutral gear first in order to properly test MOTOR_SAFE_TO_RUN\n\r");
    printf("ACTION: Put the gear switch in neutral\n\r");
    while(BSP_GPIO_Read_Pin(REVERSE_PORT, REVERSE) || BSP_GPIO_Read_Pin(FORWARD_PORT, FORWARD)) {} // Waiting until in neutral 
    getGear();
    printf("ACTION: Put the gear switch in forward\n\r");
    while(getGear() != DASH_FWD) {} // Waiting until in forward
    printf("ACTION: Hold accel pedal pressed down\n\r");
    // NOTE: Check for <= ACCEL_PEDAL_THRESHOLD + 5 since minimum for current to be sent to motor is ACCEL_PEDAL_THRESHOLD
    while(Pedals_Read(ACCELERATOR) <= ACCEL_PEDAL_THRESHOLD + 5) {} 
    printf("Now testing MOTOR_SAFE_TO_RUN\n\r");
    OSFlagPost(&BPS_Motor_Status_Flags, BPS_CHECKED | BPS_SAFE | MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_SET, &err);
    assertOSError(err);
    while(get_velocitySetpoint() != MAX_VELOCITY || get_currentSetpoint() == 0.0f) {}
    printf("ACTION: Put the gear switch in neutral");
    while(getGear() != DASH_NEU) {}


    // Testing Neutral
    printf("\n\rTesting Neutral================================\n\r");
    // Testing no current to motor when accel is pressed while in neutral
    printf("ACTION: Press the accel pedal fully\n\r");
    printf("EXPECTED: currentSetpoint stays = 0.0f\n\r");
    while(get_accelPedalPercent() < 75) {}
    printFullState();
    while(get_currentSetpoint() != 0.0f) {}
    // Testing brake 
    printf("ACTION: Press the brake pedal fully\n\r");
    printf("EXPECTED: brakePedalPercent increases, all else stays the same\n\r");
    while(get_brakePedalPercent() < 75) {}
    printFullState();
    printf("============================================\n\r");

    // Neutral to Forward Drive
    printf("\n\rTesting: Neutral -> Forward Drive==============\n\r");
    printf("ACTION: Switch to forward gear\n\r");
    printFullState();
    while(get_gear() != DASH_FWD){}
    printFullState();
    printf("============================================\n\r");

    // Testing Forward
    printf("\n\rTesting Forward=============================\n\r");
    printFullState();
    printf("ACTION: Press the accel pedal fully\n\r");
    printf("EXPECTED: currentSetpoint increases, all else stays the same\n\r");
    while(get_accelPedalPercent() < 75) {}
    printFullState();
    printf("ACTION: Keep the accel pedal fully pressed & also fully press brake pedal\n\r");
    printf("EXPECTED: currentSetpoint stays = 0.0f");
    while(get_currentSetpoint() != 0.0f) {}
    printFullState();
    printf("============================================\n\r");
    
    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Forward Drive -> Reverse Drive======\n\r");
    printf("ACTION: Switch to reverse gear\n\r");
    printFullState();
    while(get_gear() != DASH_REV){}
    printFullState();
    // Quick check to ensure velcoity setpoint is -MAX_VELOCITY
    while(get_velocitySetpoint() != -MAX_VELOCITY) {}
    printf("============================================\n\r");

    // Testing Reverse
    printf("\n\rTesting Reverse=============================\n\r");
    printFullState();
    printf("ACTION: Press the accel pedal fully\n\r");
    printf("EXPECTED: currentSetpoint increases, all else stays the same\n\r");
    while(get_accelPedalPercent() < 75) {}
    printFullState();
    printf("ACTION: Keep the accel pedal fully pressed & also fully press brake pedal\n\r");
    printf("EXPECTED: currentSetpoint stays = 0.0f");
    while(get_currentSetpoint() != 0.0f) {}
    printFullState();
    printf("============================================\n\r");

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive======\n\r");
    printf("ACTION: Switch to forward gear\n\r");
    printFullState();
    while(get_gear() != DASH_FWD){}
    printFullState();
    printf("============================================\n\r");

    // Forward Drive to Neutral Drive
    printf("\n\rTesting: Forward Drive -> Neutral Drive=========\n\r");
    printf("ACTION: Switch to neutral gear\n\r");
    printFullState();
    while(get_gear() != DASH_NEU){}
    printFullState();
    printf("============================================\n\r");

    // Reverse Drive to Neutral Drive
    printf("\n\rTesting: Reverse Drive -> Neutral Drive=========\n\r");
    printf("ACTION: Switch to reverse gear\n\r");
    printFullState();
    while(get_gear() != DASH_REV){}
    printFullState();
    printf("ACTION: Switch to neutral gear\n\r");
    while(get_gear() != DASH_NEU) {}
    printFullState();
    printf("============================================\n\r");

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