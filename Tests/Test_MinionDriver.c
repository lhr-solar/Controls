#include "os.h"
#include "Tasks.h"
#include "Minions.h"
#include <bsp.h>
#include "config.h"
#include "common.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{   
    CPU_Init();
    
    BSP_UART_Init(UART_2);
    Minion_Init();
    bool lightState=false;

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    OS_ERR e;

    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &e);
    
    // tests the reading and writing
    // while (1){
    //     char str[128];
    //     bool regen = Minion_Read_Input(REGEN_SW);
    //     bool forward = Minion_Read_Input(FOR_SW);
    //     bool reverse = Minion_Read_Input(REV_SW);
    //     bool enable = Minion_Read_Input(CRUZ_EN);
    //     bool set = Minion_Read_Input(CRUZ_ST);
    //     bool ign1 = Minion_Read_Input(IGN_1);
    //     bool ign2 = Minion_Read_Input(IGN_2);


    //     int size = sprintf(str, "regen = %d, forward = %d, reverse = %d, enable = %d, set = %d IGN1 = %d, IGN2 = %d \r", regen, forward,reverse,enable,set,ign1,ign2);
    //     BSP_UART_Write(UART_2, str, size);
        
    //     lightState=!lightState;
    //     Minion_Write_Output(BRAKELIGHT, lightState);

    //     OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &e);           
    // }

    //tests the edge cases
    bool output = Minion_Write_Output(REV_SW, lightState);
    bool input = Minion_Read_Input(BRAKELIGHT);

    printf("This should print 0: %d", input);
    printf("This should print 0: %d", output);

    lightState = !lightState;
    for(int i = 0; i < 999999; i++){}//should be half a second
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
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
}