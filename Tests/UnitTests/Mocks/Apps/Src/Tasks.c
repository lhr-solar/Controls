#include "fff.h"
#include "Tasks.h"
#include "Minions.h"

/**
 * TCBs
 */
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB DebugDump_TCB;
OS_TCB CommandLine_TCB;

// task_trace_t PrevTasks;

/**
 * Stacks
 */
// CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
// CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
// CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
// CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
// CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
// CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
// CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
// CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];

// Variables to store error codes, stored and cleared in task error assert functions
error_code_t Error_ReadCarCAN = /*READCARCAN_ERR_NONE*/ 0; // TODO: change this back to the error 
// error_code_t Error_ReadTritium = T_NONE;  // Initialized to no error
// error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

extern const pinInfo_t PININFO_LUT[]; // For GPIO writes. Externed from Minions Driver C file.

DEFINE_FAKE_VOID_FUNC(Task_Init, void*);

// DEFINE_FAKE_VOID_FUNC(Task_SendTritium, void*);

// DEFINE_FAKE_VOID_FUNC(Task_ReadCarCAN, void*);

// DEFINE_FAKE_VOID_FUNC(Task_UpdateDisplay, void*);

// DEFINE_FAKE_VOID_FUNC(Task_ReadTritium, void*);

// DEFINE_FAKE_VOID_FUNC(Task_SendCarCAN, void*);

// DEFINE_FAKE_VOID_FUNC(Task_DebugDump, void*);

// DEFINE_FAKE_VOID_FUNC(Task_CommandLine, void*);

// DEFINE_FAKE_VOID_FUNC(TaskSwHook_Init);

// DEFINE_FAKE_VOID_FUNC(EmergencyContactorOpen);

DEFINE_FAKE_VOID_FUNC(throwTaskError, error_code_t, callback_t, error_scheduler_lock_opt_t, error_recov_opt_t);


DEFINE_FAKE_VOID_FUNC(_assertOSError, OS_ERR);
