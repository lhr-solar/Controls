/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Tasks.c
 * @brief
 *
 */

#include "os_cfg_app.h"

#include "CANbus.h"
#include "Contactors.h"
#include "DebugIO.h"
#include "Display.h"
#include "Pedals.h"
#include "StatusLeds.h"

#include "IOState.h"
#include "ReadCarCAN.h"
#include "ReadTritium.h"
#include "SendTritium.h"
#include "Tasks.h"
#include "UpdateDisplay.h"
#include "daybreak_pins.h"

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
OS_TCB IOState_TCB;

task_trace_t PrevTasks;

/**
 * Stacks
 */
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];
CPU_STK IOState_Stk[TASK_IO_STATE_STACK_SIZE];

/* Controls Fault Message bits */
#define ANY_CONTROLS_FAULT_BIT     0x1 << 0 // 1 if any of the other bits are true
#define MOTOR_CONTROLLER_FAULT_BIT 0x1 << 1 // 1 if there is a ReadTritium Error
#define BPS_FAULT_BIT              0x1 << 2 // 1 if there is a BPS Trip error
// #define PEDALS_FAULT_BIT           0x1 << 3 // ""
#define READCARCAN_FAULT_BIT       0x1 << 4 // 1 if there is a ReadCarCAN Error
#define DISPLAY_FAULT_BIT          0x1 << 5 // 1 if there is an UpdateDisplay Error
#define OS_FAULT_BIT               0x1 << 6 // 1 if there is an OS Error
#define LAKSHAY_FAULT_BIT          0x1 << 7 // 1 if Lakshay's code is running

#define FAULT_MSG_DELAY            1000000

const char ERROR_MSGS[NUM_CONTROLS_ERRORS][ERRMSG_MAX_LEN] = {
    [C_ERR_NONE] = "\"N/A\"",
    // Read Tritium Errors
    [C_ERR_RTR_GENERIC] = "\"RTR_GENERIC\"",
    [C_ERR_RTR_HARDWARE_OC] = "\"MOT_HW_OC\"",
    [C_ERR_RTR_SOFTWARE_OC] = "\"MOT_SW_OC\"",
    [C_ERR_RTR_DC_BUS_OV] = "\"MOT_DC_BUS_OV\"",
    [C_ERR_RTR_HALL_SENSOR] = "\"MOT_HALLSENSR\"",
    [C_ERR_RTR_WDOG_LAST_RESET] = "\"\"",
    [C_ERR_RTR_CONFIG_READ] = "",
    [C_ERR_RTR_UNDERVOLT_LOCKOUT] = "",
    [C_ERR_RTR_DESAT_FAULT] = "",
    [C_ERR_RTR_MOTOR_OVERSPEED] = "",
    [C_ERR_RTR_INIT_FAIL] = "",
    [C_ERR_RTR_MOTOR_WDOG_TRIP] = "",
    // Send Tritium Errors
    [C_ERR_STR_GENERIC] = "",
    [C_ERR_STR_GEAR_FAULT] = "", /* Received multiple or no gear inputs */
    // Read Car CAN Errors
    [C_ERR_RCC_GENERIC] = "",
    [C_ERR_RCC_BPS_MISSED_MSG] = "",       /* Didn't receive a BPS msg in time (watchdog trip) */
    [C_ERR_RCC_PRECHARGE_MISSED_MSG] = "\"PRECHRG_MISS\"", /* Didn't receive a precharge msg in time */
    [C_ERR_RCC_BPS_TRIP] = "\"PRECHRG_MISS\"",             /* Recieved a BPS trip msg */
    [C_ERR_RCC_ACTIVE_PRECHARGE_FLT] = "", /* Received active precharge fault */
    // IO state Errors
    [C_ERR_IOS_GENERIC] = "",
    [C_ERR_IOS_IGN_FAULT] = "",
    // Update display errors
    [C_ERR_UPD_GENERIC] = "",
    [C_ERR_UPD_PARSE_COMPONENT] = "",
    [C_ERR_UPD_DRIVER] = "",
    // Special
    [C_ERR_GENERIC] = "\"ERROR :(\"",
    [C_ERR_ILLEGAL_ERROR] = "\"ILLEGAL_ERR\"", /* An error was thrown that doesn't belong */
};

// Synchronization-protected event flag group signaling BPS_SAFE, if BPS
// has been checked, & motor ready to run status
OS_FLAG_GRP BPS_Motor_Status_Flags;

/**
 * @brief Check and set error bits for CONTROLS_FAULT_MSG
 * @param app_err the Controls-defined error.
 * @param os_err the OS error.
 * @return a byte with the error bits set according to Controls' current faults
 */
static uint8_t get_fault_bits(controls_error_e app_err, OS_ERR os_err) {
    uint8_t msg = 0;

    if (app_err >= C_ERR_UPD_GENERIC)
        msg |= DISPLAY_FAULT_BIT;
    else if (app_err >= C_ERR_RCC_GENERIC)
        msg |= READCARCAN_FAULT_BIT; // Includes iostate for now
    else if (app_err >= C_ERR_RTR_GENERIC)
        msg |= MOTOR_CONTROLLER_FAULT_BIT; // Includes read and send tritium for now

    if (app_err == C_ERR_RCC_BPS_TRIP) msg |= BPS_FAULT_BIT;

    if (os_err != OS_ERR_NONE) msg |= OS_FAULT_BIT;

    if (msg != 0) msg |= ANY_CONTROLS_FAULT_BIT;

    // msg |= LAKSHAY_FAULT_BIT; // TODO: remove this when Lakshay's code is removed

    return msg;
}

/**
 * Error assertion-related functions
 */

void _assertOSError(OS_ERR err) {
    if (err != OS_ERR_NONE) {
        Status_Leds_Write(OS_FAULT_LED, ON);

        char os_err_msg[ERRMSG_MAX_LEN] = {0};
        snprintf(os_err_msg, ERRMSG_MAX_LEN, "%08X", err);

        MotorContactor_EmergencyDisable(); // Turn off all contactors
        Display_Error(ERROR_MSGS[C_ERR_NONE], os_err_msg, false);

        CANDATA_t faultmsg = {0};
        faultmsg.ID = CONTROLS_FAULT_MSG;
        faultmsg.data[0] = get_fault_bits(C_ERR_NONE, err);

        volatile static int faultLoopCount = 0;

        while (1) { // nonrecoverable
            faultLoopCount++;
            if (faultLoopCount > FAULT_MSG_DELAY) {
                faultLoopCount = 0;
                CANbus_Send_Faultstate(faultmsg, CARCAN);
            }
        }
    }
}

/**
 * @brief Assert a task error by setting the location variable and optionally
 * locking the scheduler, displaying a fault screen (if nonrecoverable), jumping
 * to a callback function, and entering an infinite loop. Called by
 * task-specific error-assertion functions that are also responsible for setting
 * the error variable.
 * @param error_code the enum for the specific error that happened
 * @param error_callback a callback function to a handler for that specific
 * error (NULL is permissible),
 * @param lock_scheduler whether or not to lock the scheduler to ensure the
 * error is handled immediately
 * @param recovery whether or not to kill the motor, display the fault
 * screen, and enter an infinite while loop
 */
void throwTaskError(controls_error_e error_code, bool is_evac_needed, callback_t error_callback,
                    error_scheduler_opt_e lock_scheduler, error_recovery_opt_e recovery) {
    if (error_code == C_ERR_NONE) return;

    OS_ERR err;
    // OS_OPT_POST_NO_SCHED option is passed to make not scheduling point uwu
    OSFlagPost(&BPS_Motor_Status_Flags, MOTOR_SAFE_TO_RUN,
               OS_OPT_POST_FLAG_CLR | OS_OPT_POST_NO_SCHED, &err);
    assertOSError(err);

    Status_Leds_Write(CONTROLS_FAULT_LED, ON);

    // Prevent other tasks from interrupting the handling of important
    // (includes all nonrecoverable) errors
    if (lock_scheduler == OPT_LOCK_SCHED || recovery == OPT_NONRECOV) {
        OSSchedLock(&err);
        assertOSError(err);
    }

    if (recovery == OPT_NONRECOV) {
        MotorContactor_EmergencyDisable();
        // Needs to happen before callback so that tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
        Display_Error(ERROR_MSGS[error_code], ERROR_MSGS[OS_ERR_NONE], is_evac_needed);
    }

    // Run a handler for this error if specified
    if (error_callback != NULL) {
        error_callback();
    }

    // Send Controls fault message over Car CAN
    CANDATA_t faultmsg = {0};
    faultmsg.ID = CONTROLS_FAULT_MSG;
    faultmsg.data[0] = get_fault_bits(error_code, OS_ERR_NONE);

    CANDATA_t motormsg = {0};
    motormsg.ID = MOTOR_CONTROLLER_SAFE;
    motormsg.data[0] = 0;

    CANbus_Send_Faultstate(faultmsg, CARCAN);
    CANbus_Send_Faultstate(motormsg, CARCAN);

    if (recovery == OPT_NONRECOV) { // Enter an infinite while loop
        while (1) {
            delay_ms(500);
            Status_Leds_Toggle(CONTROLS_FAULT_LED);
            Status_Leds_Toggle(DASH_HEARTBEAT_LED);
            CANbus_Send_Faultstate(faultmsg, CARCAN);
            CANbus_Send_Faultstate(motormsg, CARCAN);
        }
    }

    // only reaches here is fault is recoverable
    if (lock_scheduler == OPT_LOCK_SCHED) {
        Status_Leds_Write(CONTROLS_FAULT_LED, OFF);
        OSSchedUnlock(&err);
        // Don't err out if scheduler is still locked because of a timer
        // callback; but we don't plan to lock more than one level deep
        if (err != OS_ERR_SCHED_LOCKED || OSSchedLockNestingCtr > 1) {
            assertOSError(err);
        }
    }
}

/**
 * @brief Hook that's called every context switch
 *
 * This function will append the task being switched out to the task trace if
 * and only if:
 *      1. It's not a task created automatically by the RTOS
 *      2. It's not the previously recorded task (a long running task
 * interrupted by the tick task will only show up once) This function will
 * overwrite tasks that have been in the trace for a while, keeping only the 8
 * most recent tasks
 */
void App_OS_TaskSwHook(void) {
    OS_TCB *cur = OSTCBCurPtr;
    uint32_t idx = PrevTasks.index;
    if (cur == &OSTickTaskTCB) return;       // Ignore the tick task
    if (cur == &OSIdleTaskTCB) return;       // Ignore the idle task
    if (cur == &OSTmrTaskTCB) return;        // Ignore the timer task
    if (cur == &OSStatTaskTCB) return;       // Ignore the stat task
    if (cur == PrevTasks.tasks[idx]) return; // Don't record the same task again
    if (++idx == TASK_TRACE_LENGTH) idx = 0;
    PrevTasks.tasks[idx] = cur;
    PrevTasks.index = idx;
}

void TaskSwHook_Init(void) {
    PrevTasks.index = TASK_TRACE_LENGTH - 1; // List starts out empty
    OS_AppTaskSwHookPtr = App_OS_TaskSwHook;
}

// Initialize BPS & Motor Event Flag Group
void BPSMotorFlags_Init(void) {
    OS_ERR err;
    OSFlagCreate(&BPS_Motor_Status_Flags, "BPS_Motor_Status_Flags", 0, &err);
    assertOSError(err);
}
