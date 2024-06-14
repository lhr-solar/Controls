/**
 * @file Tasks.c
 *
 * # Task Switch Hook Implementation Details
 * The task switch hook is a function that is called every time the RTOS
 * switches from one task to another. This is useful for debugging, as it allows
 * us to see which tasks were running when a fault occurred.
 *
 * This function will overwrite tasks that have been in the trace for a while,
 * keeping only the TASK_TRACE_LENGTH most recent tasks. It's essentially a fifo
 * that loops in on itself, so wherever prev_tasks.index is pointing is the
 * newest task in the trace, and the task at prev_tasks.index + 1 is the oldest
 * task in the trace.
 *
 * @image xml TaskTraceDiagram.png
 *
 * # Assert OS Error
 * This macro is defined differently depending on whether or not
 * DEBUG is defined. If DEBUG is defined, the macro will print the file and line
 * number of the error. If DEBUG is not defined, the macro will simply call
 * AssertOsError(err). If err is not OS_ERR_NONE, preventative measures will be
 * taken (such as calling EmergencyContactorOpen()).
 */

#include "Tasks.h"

#include "CanBus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"
#include "ReadCarCan.h"
#include "ReadTritium.h"
#include "UpdateDisplay.h"
#include "os.h"

/**
 * TCBs
 */
OS_TCB init_tcb;
OS_TCB send_tritium_tcb;
OS_TCB read_car_can_tcb;
OS_TCB update_display_tcb;
OS_TCB read_tritium_tcb;
OS_TCB send_car_can_tcb;
OS_TCB debug_dump_tcb;
OS_TCB command_line_tcb;

TaskTrace prev_tasks;

/**
 * Stacks
 */
CPU_STK init_stk[TASK_INIT_STACK_SIZE];
CPU_STK send_tritium_stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK read_car_can_stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK update_display_stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
CPU_STK read_tritium_stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK send_car_can_stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK debug_dump_stk[TASK_DEBUG_DUMP_STACK_SIZE];
CPU_STK command_line_stk[TASK_COMMAND_LINE_STACK_SIZE];

// Variables to store error codes, stored and cleared in task error assert
// functions
ErrorCode error_read_car_can = 0;      // TODO: change this back to the error
ErrorCode error_read_tritium = kNone;  // Initialized to no error
ErrorCode error_update_display = kUpdateDisplayErrNone;

extern const PinInfo
    kPininfoLut[];  // For GPIO writes. Externed from Minions Driver C file.

/**
 * Error assertion-related functions
 */
void AssertOsError(volatile OS_ERR err) {
    OS_ERR err2 = 0;
    if (err != OS_ERR_NONE) {
        OSSchedLock(&err2);
        EmergencyContactorOpen();  // Turn off contactors and turn on the
                                   // brakelight to indicate an emergency
        DisplayFault(err);         // Display the location and error code
        printf("%d\n\r", err);
        while (1) {}  // nonrecoverable
    }
}

/**
 * @brief Assert a task error by locking the scheduler (if necessary),
 * displaying a fault screen, and jumping to the error's specified callback
 * function. Called by task-specific error-assertion functions that are also
 * responsible for setting the error variable.
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific
 * error,
 * @param lockSched whether or not to lock the scheduler to ensure the error is
 * handled immediately. Only applicable for recoverable errors- nonrecoverable
 * errors will always lock
 * @param nonrecoverable whether or not to kill the motor, display the fault
 * screen, and enter an infinite while loop
 */
void ThrowTaskError(ErrorCode error_code, Callback error_callback,
                    ErrorSchedulerLockOpt lock_sched,
                    ErrorRecovOpt nonrecoverable) {
    OS_ERR err = 0;

    if (error_code == 0) {  // Exit if there is no error
        return;
    }

    if (lock_sched == kOptLockSched ||
        nonrecoverable ==
            kOptNonrecov) {  // Prevent other tasks from interrupting the
                             // handling of important (includes all
                             // nonrecoverable) errors
        OSSchedLock(&err);
        ASSERT_OS_ERROR(err);
    }

    if (nonrecoverable == kOptNonrecov) {
        EmergencyContactorOpen();
        DisplayFault(error_code);  // Needs to happen before callback so that
                                   // tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
        printf("%d\n\r", error_code);
        UpdateDisplayClearQueue();  // Clear message queue to ensure no other
                                    // commands overwrite the error screen
    }

    if (error_callback != NULL) {
        error_callback();  // Run a handler for this error that was specified in
                           // another task file
    }

    if (nonrecoverable == kOptNonrecov) {  // Enter an infinite while loop
        while (1) {}
    }

    if (lock_sched == kOptLockSched) {  // Only happens on recoverable errors
        OSSchedUnlock(&err);
        // Don't err out if scheduler is still locked because of a timer
        // callback
        if (err != OS_ERR_SCHED_LOCKED ||
            OSSchedLockNestingCtr >
                1) {  // But we don't plan to lock more than one level deep
            ASSERT_OS_ERROR(err);
        }
    }
}

/**
 * @brief For use in error handling: opens array and motor precharge bypass
 * contactor and turns on additional brakelight to signal that a critical error
 * happened.
 */
void EmergencyContactorOpen() {
    // Array motor kill
    BspGpioWritePin(CONTACTORS_PORT, MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN,
                    OFF);
    BspGpioWritePin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, OFF);

    // Turn additional brakelight on to indicate critical error
    BspGpioWritePin(kPininfoLut[kBrakeLight].port,
                    kPininfoLut[kBrakeLight].pin_mask, true);
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
void AppOsTaskSwHook(void) {
    OS_TCB *cur = OSTCBCurPtr;
    uint32_t idx = prev_tasks.index;
    if (cur == &OSTickTaskTCB) {
        return;  // Ignore the tick task
    }
    if (cur == &OSIdleTaskTCB) {
        return;  // Ignore the idle task
    }
    if (cur == &OSTmrTaskTCB) {
        return;  // Ignore the timer task
    }
    if (cur == &OSStatTaskTCB) {
        return;  // Ignore the stat task
    }
    if (cur == prev_tasks.tasks[idx]) {
        return;  // Don't record the same task again
    }
    if (++idx == TASK_TRACE_LENGTH) {
        idx = 0;
    }
    prev_tasks.tasks[idx] = cur;
    prev_tasks.index = idx;
}

void TaskSwHookInit(void) {
    prev_tasks.index = TASK_TRACE_LENGTH - 1;  // List starts out empty
    OS_AppTaskSwHookPtr = AppOsTaskSwHook;
}
