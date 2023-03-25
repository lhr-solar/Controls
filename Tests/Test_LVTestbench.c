#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include "Minions.h"
#include "Contactors.h"
#include "Pedals.h"
#include "BSP_UART.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

const char *show(bool b) {
    return b ? "on\r" : "off\r";
}

void Task1(void *arg) {   

    OS_ERR err;
    Minion_Error_t me;
    CPU_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    Contactors_Init();
    Minion_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();

    bool ign1, ign2, regen, forward, reverse, cruise_en, cruise_set;
    // Contactors_Set(ARRAY_CONTACTOR,true,true);
    uint8_t i = 0; 
    bool state = true;
    for (;;i++) {
        if(i % 10 == 0) {
            state = !state;
            Contactors_Set(ARRAY_CONTACTOR,state,true);
        }
        ign1       = !Minion_Read_Input(   IGN_1, &me);
        ign2       = !Minion_Read_Input(   IGN_2, &me);
        regen      = Minion_Read_Input(REGEN_SW, &me);
        forward    = Minion_Read_Input(  FOR_SW, &me);
        reverse    = Minion_Read_Input(  REV_SW, &me);
        cruise_en  = Minion_Read_Input( CRUZ_EN, &me);
        cruise_set = Minion_Read_Input( CRUZ_ST, &me);
        printf("IGN1: %s\nIGN2: %s\nRegen: %s\nForward: %s\nReverse: %s\nCruise Enable: %s\nCruise Set: %s\n\n",
                show(ign1), show(ign2), show(regen), show(forward), show(reverse), show(cruise_en), show(cruise_set)
        );
        printf("Brake percent: %d, voltage: %d mv\r\n", Pedals_Read(BRAKE), BSP_ADC_Get_Millivoltage(Brake_ADC));
        printf("Accel percent:%d, voltage: %dmv \r\n",Pedals_Read(ACCELERATOR), BSP_ADC_Get_Millivoltage(Accelerator_ADC));
        for (int j=0; j<100; j++) {
            Minion_Write_Output(BRAKELIGHT, Pedals_Read(BRAKE) > 10, &me);
            OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
        }
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);
    assertOSError(OS_MAIN_LOC, err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)2,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
}