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

/**
 * String array holding all the error message string for all application errors in the
 * controls code.
 *
 * If the `controls_error_e` enum is changed, this array should also be
 * modified accordingly. When the respective controls error is asserted, the corresponding
 * string will be displayed on the fault page, with the exception of the `C_ERR_RTR_MULTIPLE`
 * because in that case we want to see the bitmap.
 */

const char ERROR_MSGS[NUM_CONTROLS_ERRORS][ERRMSG_MAX_LEN] = {
    // Generic errors are placeholder errors for debugging purposes and should eventually
    // become individual errors themselves

    [C_ERR_NONE]                     = "\"N/A\"",           /* No error :) */
    // Read Tritium Errors
    [C_ERR_RTR_GENERIC]              = "\"RTR_GENERIC\"",   /* Generic placeholder error */
    [C_ERR_RTR_HARDWARE_OC]          = "\"MOT_HW_OC\"",
    [C_ERR_RTR_SOFTWARE_OC]          = "\"MOT_SW_OC\"",
    [C_ERR_RTR_DC_BUS_OV]            = "\"MOT_DC_BUS_OV\"",
    [C_ERR_RTR_HALL_SENSOR]          = "\"MOT_HALLSENSR\"",
    [C_ERR_RTR_WDOG_LAST_RESET]      = "\"MOT_DOG_LREST\"",
    [C_ERR_RTR_CONFIG_READ]          = "\"MOT_CONFIG_RD\"",
    [C_ERR_RTR_UNDERVOLT_LOCKOUT]    = "\"MOT_UNDERV_LK\"",
    [C_ERR_RTR_DESAT_FAULT]          = "\"MOT_DESAT_FLT\"",
    [C_ERR_RTR_MOTOR_OVERSPEED]      = "\"MOT_OVERSPEED\"",
    [C_ERR_RTR_INIT_FAIL]            = "\"RTR_INIT_FAIL\"", /* TODO: WHAT IS THIS?? */
    [C_ERR_RTR_MOTOR_WDOG_TRIP]      = "\"RTR_WDOG_TRIP\"",
    [C_ERR_RTR_MULTIPLE]             = "\"RTR_MULTI_ERR\"", /* Should never actually display this */
    // Send Tritium Errors
    [C_ERR_STR_GENERIC]              = "\"STR_GENERIC\"",   /* Generic placeholder error */
    [C_ERR_STR_GEAR_FAULT]           = "\"STR_GEAR_FLT\"",  /* Received multiple or no gear inputs */
    // Read Car CAN Errors
    [C_ERR_RCC_GENERIC]              = "\"RCC_GENERIC\"",   /* Generic placeholder error */
    [C_ERR_RCC_BPS_MISSED_MSG]       = "\"BPS_MISS\"",      /* Didn't receive a BPS msg in time */
    [C_ERR_RCC_PRECHARGE_MISSED_MSG] = "\"PRECHG_MISS\"",   /* Didn't receive prechrg msg in time */
    [C_ERR_RCC_BPS_TRIP]             = "\"BPS_TRIP\"",      /* Recieved a BPS trip msg */
    [C_ERR_RCC_ACTIVE_PRECHARGE_FLT] = "\"ACT_PRECH_FLT\"", /* Received active precharge fault */
    [C_ERR_RCC_PRECHARGE_TMOUT_MOT]  = "\"PRECH_MTOUT\"",   /* Received active precharge timeout fault for motor */
    [C_ERR_RCC_PRECHARGE_TMOUT_ARR]  = "\"PRECH_ARTOUT\"",  /* Received active precharge timeout fault for array */
    [C_ERR_RCC_PRECHARGE_MOT_SENSE_FLT] = "\"PRECH_MSENSE\"", /* Received precharge motor sense fault */
    [C_ERR_RCC_PRECHARGE_ARR_PRE_SENSE_FLT] = "\"PRECH_ASENSE\"", /* Received precharge array pre sense fault */
    [C_ERR_RCC_PRECHARGE_MOT_PRE_SENSE_FLT] = "\"PRECH_MPSENSE\"", /* Received precharge motor pre sense fault */

    // IO state Errors
    [C_ERR_IOS_GENERIC]              = "\"IOS_GENERIC\"",   /* Generic placeholder error */
    [C_ERR_IOS_IGN_FAULT]            = "\"IOS_IGN_FLT\"",   /* Ignition unstable for too long */
    // Update display errors
    [C_ERR_UPD_GENERIC]              = "\"UPD_GENERIC\"",   /* Generic placeholder error */
    [C_ERR_UPD_PARSE_COMPONENT]      = "\"UPD_PARSE_COM\"", /* Error in parsing a componenet */
    [C_ERR_UPD_DRIVER]               = "\"UPD_DRIVR_ERR\"", /* Error propogating from driver */
    // Special
    [C_ERR_GENERIC]                  = "\"GENERIC_ERROR\"", /* Generic placeholder error */
    [C_ERR_ILLEGAL_ERROR]            = "\"ILLEGAL_ERR\"",   /* Error thrown in illegal context */
};

// Synchronization-protected event flag group signaling BPS_SAFE, if BPS
// has been checked, & motor ready to run status
OS_FLAG_GRP BPS_Motor_Status_Flags;

// The defined bits in the flag group
const uint8_t ALLOWED_BITS = BPS_SAFE | BPS_CHECKED | MOTOR_SAFE_TO_RUN;

/**
 * @brief Check and set error bits for CONTROLS_FAULT_MSG
 * @param app_err the Controls-defined error.
 * @param os_err the OS error.
 * @return a byte with the error bits set according to Controls' current faults
 */
static uint8_t get_fault_bits(controls_error_e app_err, OS_ERR os_err) {
    uint8_t msg = 0;

    if (app_err != C_ERR_NONE) {
        msg |= ANY_CONTROLS_FAULT_BIT;

        if (app_err == C_ERR_RCC_BPS_TRIP) msg |= BPS_FAULT_BIT;

        if (app_err < C_ERR_RCC_GENERIC) //  Incl read and send tritium
            msg |= MOTOR_CONTROLLER_FAULT_BIT;
        else if (app_err < C_ERR_UPD_GENERIC) // Incl rcc and iostate
            msg |= READCARCAN_FAULT_BIT;
        else if (app_err < C_ERR_GENERIC)
            msg |= DISPLAY_FAULT_BIT;
        // Generic controls errors do not affect the bits other than
        // the general cotnrols fault bit
    }

    if (os_err != OS_ERR_NONE) {
        msg |= ANY_CONTROLS_FAULT_BIT;
        msg |= OS_FAULT_BIT;
    }

    msg |= LAKSHAY_FAULT_BIT; // TODO: remove this when Lakshay's code is removed

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
        Display_Error(ERROR_MSGS[C_ERR_NONE], os_err_msg, false, CAN_NONE_BPS);

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

static void setDisplayErrorScreen(controls_error_e error_code, bool is_evac_needed, BPSFaultErr_e bps_err) {
    if (error_code == C_ERR_RTR_MULTIPLE) {
        char err_msg_multiple[ERRMSG_MAX_LEN] = {0};
        snprintf(err_msg_multiple, ERRMSG_MAX_LEN, "\"MOCO_%03X\"",
                    Motor_Error_Get() & 0xFFF);
        Display_Error(err_msg_multiple, ERROR_MSGS[OS_ERR_NONE], is_evac_needed, bps_err);
    } else {
        Display_Error(ERROR_MSGS[error_code], ERROR_MSGS[OS_ERR_NONE], is_evac_needed, bps_err);
    }
}

#define ERROR_CAN_DELAY_MS 500
#define ERROR_DISPLAY_UPDATE_COUNT (1000 / ERROR_CAN_DELAY_MS)

/**
 * @brief Assert a task error by setting the location variable and optionally
 * locking the scheduler, displaying a fault screen (if nonrecoverable), jumping
 * to a callback function, and entering an infinite loop. Called by
 * task-specific error-assertion functions that are also responsible for setting
 * the error variable.
 * @param error_code the enum for the specific error that happened
 * @param is_evac_needed whether evac is required, it will be recommended regardless.
 * @param error_callback a callback function to a handler for that specific
 * error (NULL is permissible),
 * @param lock_scheduler whether or not to lock the scheduler to ensure the
 * error is handled immediately
 * @param recovery whether or not to kill the motor, display the fault
 * screen, and enter an infinite while loop
 */
void throwTaskError(controls_error_e error_code, bool is_evac_needed, callback_t error_callback,
                    error_scheduler_opt_e lock_scheduler, error_recovery_opt_e recovery, BPSFaultErr_e bps_err) {

    if (error_code == C_ERR_NONE) return;

    // Set the motor safe to run bit to false and don't make it a scheduling point
    // This avoids context switching while we're in an error
    MotorStatus_ModifyBits(MOTOR_SAFE_TO_RUN, false, !OS_FLAG_SCHED_POINT);

    OS_ERR err;

    Status_Leds_Write(CONTROLS_FAULT_LED, ON);


    // Prevent other tasks from interrupting the handling of important
    // (includes all nonrecoverable) errors
    if (lock_scheduler == OPT_LOCK_SCHED || recovery == OPT_NONRECOV) {
        OSSchedLock(&err);
        assertOSError(err);
    }

    if (recovery == OPT_NONRECOV) {
        MotorContactor_EmergencyDisable();
        setDisplayErrorScreen(error_code, is_evac_needed, bps_err);
    }

    // Run a handler for this error if specified
    if (error_callback != NULL) {
        error_callback();
    }

    // Send Controls fault message over Car CAN
    CANDATA_t faultmsg = {0};
    faultmsg.ID        = CONTROLS_FAULT_MSG;
    faultmsg.data[0]   = get_fault_bits(error_code, OS_ERR_NONE);

    CANDATA_t motormsg = {0};
    motormsg.ID        = MOTOR_CONTROLLER_SAFE;

    motormsg.data[0]   = 0;
    motormsg.data[0] |= 0x2; // Bit 1 of motor message

    CANbus_Send_Faultstate(faultmsg, CARCAN);
    CANbus_Send_Faultstate(motormsg, CARCAN);

    // set motor and array positions off
    CANDATA_t iostatemsg = {0};
    iostatemsg.ID = IO_STATE;
    iostatemsg.data[0] |= SWITCH_BITMAP_IGN_1_ARRAY(0);
    iostatemsg.data[0] |= SWITCH_BITMAP_IGN_2_MOTOR(0);

    volatile int displayUpdateCount = 0;

    if (recovery == OPT_NONRECOV) { // Enter an infinite while loop
        while (1) {

            delay_ms(500);
            Status_Leds_Toggle(CONTROLS_FAULT_LED);
            Status_Leds_Toggle(DASH_HEARTBEAT_LED);
            CANbus_Send_Faultstate(faultmsg, CARCAN);
            CANbus_Send_Faultstate(motormsg, CARCAN);
            CANbus_Send_Faultstate(iostatemsg, CARCAN);
            displayUpdateCount++;
            // Updates the display every ERROR_DISPLAY_UPDATE_COUNT iterations
            // Did this due to the display sometimes browning out and resetting when voltage drops in fault
            if(displayUpdateCount >= ERROR_DISPLAY_UPDATE_COUNT) {
                displayUpdateCount = 0;
                //setDisplayErrorScreen(error_code, is_evac_needed);
            }
        }
    }
    else{

        Status_Leds_Write(CONTROLS_FAULT_LED, OFF);

        // Only unlock the scheduler if we locked it
        if(lock_scheduler == OPT_LOCK_SCHED){
            OSSchedUnlock(&err);
            // Don't err out if scheduler is still locked because of a timer
            // callback; but we don't plan to lock more than one level deep
            if (err != OS_ERR_SCHED_LOCKED || OSSchedLockNestingCtr > 1) {
                assertOSError(err);
            }
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

/**
 * @brief A generic wrapper for pending on BPS_Motor_Status_Flags.
 * @param bits these are the bits to pend on to be set.
 * @param blocking whether to block the thread or not.
 */
OS_ERR MotorStatus_Wait(uint8_t bits, bool blocking) {
    // Validate bit input
    if ((bits & ~ALLOWED_BITS) != 0) return OS_ERR_OPT_INVALID;

    OS_ERR err;
    OS_OPT block_opt = blocking ? OS_OPT_PEND_BLOCKING : OS_OPT_PEND_NON_BLOCKING;
    OSFlagPend(&BPS_Motor_Status_Flags, bits, 0,
               OS_OPT_PEND_FLAG_SET_ALL | block_opt , NULL, &err);

    // Nonblocking and not all the specified bits are set
    if(!blocking && err == OS_ERR_PEND_WOULD_BLOCK){
        return OS_ERR_PEND_WOULD_BLOCK;
    }

    // If there errors other than OS_ERR_NONE, assert them, should never return from this
    if(err != OS_ERR_NONE){
        assertOSError(err);
    }

    
    return err;
}

/**
 * @brief Fucntion to get the flags directly from BPS_Motor_Status_Flags in a critical section.
 * @return a copy of the OS_FLAGS from BPS_Motor_Status_Flags.
 */
OS_FLAGS MotorStatus_GetBits() {
    OS_FLAGS current_flags;

    // In cpu.h, it says that the cpu status register variable may need to be set
    // and that it should be set after local variables are declared.
    // After that, enter a critical section; interrupts are disabled after saving status.
    // See cpu.h line 250ish
    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();
    current_flags = BPS_Motor_Status_Flags.Flags;
    OS_CRITICAL_EXIT();

    return current_flags;
}

/**
 * @brief Function to modify the bitmap of flags within BPS_Motor_Status_Flags. 
 * 
 * @param bits these are the bits to either set or clear. Must be a valid combination.
 * @param state whether or not to clear (false) or to set (true) the given bits.
 * @param allow_sched whether there may be a scheduling point or not. (i.e. whether the option 
 *                    OS_OPT_POST_NO_SCHED should be included)
 * 
 * @return a bool value representing if the modification was successful.
 */
bool MotorStatus_ModifyBits(uint8_t bits, bool state, bool allow_sched) {
    // Validate bit input
    if ((bits & ~ALLOWED_BITS) != 0) return false;

    OS_OPT set_opt = state ? OS_OPT_POST_FLAG_SET : OS_OPT_POST_FLAG_CLR;
    OS_OPT sched_opt = allow_sched ? 0 : OS_OPT_POST_NO_SCHED;
    OS_ERR err;
    OS_FLAGS flags = OSFlagPost(&BPS_Motor_Status_Flags, bits, set_opt | sched_opt, &err);
    assertOSError(err);

    return (flags & bits) == (state ? bits : 0);
}
