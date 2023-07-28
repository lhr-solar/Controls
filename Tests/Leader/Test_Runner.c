/**
 * @file Test_Runner.c
 * @brief Test runner. Main code executes here.
 */

#include "Test_Runner.h"

static OS_TCB TestTCB;
static CPU_STK TestStartTaskStk[DEFAULT_STACK_SIZE];

static const char* const verify_str = "Verify\n";
static const char* const continue_str = "Continue\n";

void __attribute__((unused)) Verify(bool blocking){
    printf(verify_str);
    
    char input[13];
    if(blocking){
        while(strcmp(input, continue_str))
            BSP_UART_Read(UART_2, input);
    }
}

static inline void Init();
static inline void Start();

int main(){
    Init();
    Start();
    
    while(1);
    
    return 0;
}

#if TEST_USE_RTOS
static inline void Init(){
    OS_ERR err;
    
    CPU_Init();
    OSInit(&err);

    // Disable interrupts
    __disable_irq();
    
    // Initialize needed drivers
    BSP_UART_Init(UART_2);

    // Specific test setup instructions
    Test_Setup();

    // Enable interrupts
    __enable_irq();
    assertOSError(OS_MAIN_LOC, err); // Uses printf so must be after UART init

    TaskSwHook_Init();

    // Start SysTick
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
}

static void Test_Start_Task(){
    Test_Start();
    printf("End Test\n");
}

static inline void Start(){
    // Spawn the test task
    OSTaskCreate(
        (OS_TCB *)&TestTCB,
        (CPU_CHAR *)"Test Task",
        (OS_TASK_PTR)Test_Start_Task,
        (void *)NULL,
        (OS_PRIO)TASK_TEST_PRIO,
        (CPU_STK *)TestStartTaskStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);
    
    // Start OS
    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);
}

#else
static inline void Init(){
    // Disable interrupts
    __disable_irq();
    
    // Initialize needed drivers
    BSP_UART_Init(UART_2);

    // Specific test setup instructions
    Test_Setup();

    // Enable interrupts
    __enable_irq();

    return;
}

static inline void Start(){
    Test_Start();
}

#endif
