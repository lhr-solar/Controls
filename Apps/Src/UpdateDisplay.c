/**
 * @file UpdateDisplay.c
 *
 * The task implements a queue of command structures. Tasks that wish to
 * write to the display call one of the wrapper functions exposed in the public
 * interface. Internally, all of these functions construct a command structure
 * and then call UpdateDisplayPutNext(), which places the command structure in
 * the queue and notifies UpdateDisplay. Another internal function,
 * UpdateDisplayPopNext(), gets called by UpdateDisplay. It blocks on a
 * semaphore until the queue is not empty, and then grabs the next command
 * structure. It then parses the command and sends it out over UART. Check these
 * functions in UpdateDisplay.c for more information.
 *
 */

#include "UpdateDisplay.h"

#include <math.h>

#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Tasks.h"
#include "common.h"
#include "os.h"

#define NUM_COMP_STRINGS 15

/**
 * @brief Size of the display queue
 */
#define DISP_Q_SIZE 10

#define FIFO_TYPE DisplayCmd
#define FIFO_SIZE DISP_Q_SIZE
#define FIFO_NAME DispFifo
#include "fifo.h"

/**
 * @brief Number of times to reset before displaying the fault screen
 */
#define RESTART_THRESHOLD 3

DispFifo msg_queue;

static OS_SEM
    display_q_sem4;  // counting semaphore for queue message availability
static OS_MUTEX display_q_mutex;  // mutex to ensure thread safety when
                                  // writing/reading to queue

/**
 * @brief Check for a display error and assert it if it exists.
 * Stores the error code, calls the main assertion function
 * and runs a callback function as a handler to restart the display and clear
 * the queue.
 * @param   err variable with display error codes
 */
static void assertUpdateDisplayError(UpdateDisplayError err);

/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum {
    // Boolean components
    kArray = 0,
    kMotor,
    // Non-boolean components
    kVelocity,
    kAccelMeter,
    kSoc,
    kSuppBatt,
    kCruiseSt,
    kRegenSt,
    kGear,
    // Fault code components
    kOsCode,
    kFaultCode
} Component;

const char* comp_strings[NUM_COMP_STRINGS] = {
    // Boolean components
    "arr", "mot",
    // Non-boolean components
    "vel", "accel", "soc", "supp", "cruiseSt", "rbsSt", "gear",
    // Fault code components
    "oserr", "faulterr"};

UpdateDisplayError UpdateDisplayInit() {
    OS_ERR err = 0;
    DispFifoRenew(&msg_queue);
    OSMutexCreate(&display_q_mutex, "Display mutex", &err);
    ASSERT_OS_ERROR(err);
    OSSemCreate(&display_q_sem4, "Display sem4", 0, &err);
    ASSERT_OS_ERROR(err);

    UpdateDisplayError ret = UpdateDisplaySetPage(kInfo);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT,  // NOLINT
                  &err);  // Wait >215ms so errors will show on the display
    ASSERT_OS_ERROR(err);
    return ret;
}

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on semaphore and mutex to ensure that:
 *  1) queue has messages to send (signaled by semaphore)
 *  2) queue is not currently being written to by a separate thread (mutex)
 * @returns UpdateDisplayError
 */
static UpdateDisplayError updateDisplayPopNext() {
    DisplayCmd cmd;

    OS_ERR err = 0;
    CPU_TS ticks = 0;

    OSSemPend(&display_q_sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    ASSERT_OS_ERROR(err);

    OSMutexPend(&display_q_mutex, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    ASSERT_OS_ERROR(err);

    bool result = DispFifoGet(&msg_queue, &cmd);
    OSMutexPost(&display_q_mutex, OS_OPT_POST_ALL, &err);
    ASSERT_OS_ERROR(err);

    if (!result) {
        assertUpdateDisplayError(kUpdateDisplayErrFifoPop);
        return kUpdateDisplayErrFifoPop;
    }

    // Assert a display driver error code if the send fails, else assert that
    // there's no error
    assertUpdateDisplayError(DisplaySend(cmd) ? kUpdateDisplayErrDriver
                                              : kUpdateDisplayErrNone);
    return kUpdateDisplayErrNone;
}

/**
 * @brief Puts a new display message in the queue. Pends on mutex to ensure
 * threadsafe memory access and signals semaphore upon successful fifoPut.
 * @returns UpdateDisplayError
 */
static UpdateDisplayError updateDisplayPutNext(DisplayCmd cmd) {
    CPU_TS ticks = 0;
    OS_ERR err = 0;

    OSMutexPend(&display_q_mutex, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    ASSERT_OS_ERROR(err);

    bool success = DispFifoPut(&msg_queue, cmd);

    OSMutexPost(&display_q_mutex, OS_OPT_POST_ALL, &err);
    ASSERT_OS_ERROR(err);

    if (success) {
        OSSemPost(&display_q_sem4, OS_OPT_POST_ALL, &err);
        ASSERT_OS_ERROR(err);
    } else {
        assertUpdateDisplayError(kUpdateDisplayErrFifoPut);
        return kUpdateDisplayErrNone;
    }

    return kUpdateDisplayErrNone;
}

/**
 * @brief Several elements on the display do not update their
 * state until a touch/click event is triggered. This includes the
 * blinkers, gear selector, cruise control and regen braking indicator.
 * @returns UpdateDisplayError
 */
static UpdateDisplayError updateDisplayRefresh() {
    DisplayCmd refresh_cmd = {.comp_or_cmd = "click",
                              .attr = NULL,
                              .op = NULL,
                              .num_args = 2,
                              .arg_types = {kIntArg, kIntArg},
                              {{.num = 0}, {.num = 1}}};

    UpdateDisplayError ret = updateDisplayPutNext(refresh_cmd);
    return ret;
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return UpdateDisplayError
 */
static UpdateDisplayError updateDisplaySetComponent(Component comp,
                                                    uint32_t val) {
    UpdateDisplayError ret = kUpdateDisplayErrNone;

    // For components that are on/off
    if (comp <= kMotor && val <= 1) {
        DisplayCmd vis_cmd = {
            .comp_or_cmd = "vis",
            .attr = NULL,
            .op = NULL,
            .num_args = 2,
            .arg_types = {kStrArg, kIntArg},
            {{.str = (char*)comp_strings[comp]}, {.num = val}}};

        ret = updateDisplayPutNext(vis_cmd);
        return ret;
    }
    // For components that have a non-boolean value
    if (comp > kMotor) {
        DisplayCmd set_cmd = {.comp_or_cmd = (char*)comp_strings[comp],
                              .attr = "val",
                              .op = "=",
                              .num_args = 1,
                              .arg_types = {kIntArg},
                              {{.num = val}}};

        ret = updateDisplayPutNext(set_cmd);
        return ret;
    }
    assertUpdateDisplayError(kUpdateDisplayErrParseComp);
    return kUpdateDisplayErrParseComp;

    return kUpdateDisplayErrNone;
}

UpdateDisplayError UpdateDisplaySetPage(Page page) {
    DisplayCmd pg_cmd = {.comp_or_cmd = "page",
                         .attr = NULL,
                         .op = NULL,
                         .num_args = 1,
                         .arg_types = {kIntArg},
                         {{.num = page}}};

    UpdateDisplayError ret = updateDisplayPutNext(pg_cmd);
    return ret;
}

/* WRAPPERS */
UpdateDisplayError UpdateDisplaySetSoc(
    uint8_t percent) {  // Integer percentage from 0-100

    UpdateDisplayError ret = updateDisplaySetComponent(kSoc, percent);
    if (ret != kUpdateDisplayErrNone) {
        return ret;
    }

    ret = updateDisplayRefresh();
    return ret;
}

UpdateDisplayError UpdateDisplaySetSbpv(uint32_t mv) {
    UpdateDisplayError ret = updateDisplaySetComponent(kSuppBatt, mv / 100);
    if (ret != kUpdateDisplayErrNone) {
        return ret;
    }

    ret = updateDisplayRefresh();
    return ret;
}

UpdateDisplayError UpdateDisplaySetVelocity(uint32_t mph_tenths) {
    UpdateDisplayError ret = updateDisplaySetComponent(kVelocity, mph_tenths);
    return ret;
}

UpdateDisplayError UpdateDisplaySetAccel(uint8_t percent) {
    UpdateDisplayError ret = updateDisplaySetComponent(kAccelMeter, percent);
    return ret;
}

UpdateDisplayError UpdateDisplaySetArray(bool state) {
    UpdateDisplayError ret = updateDisplaySetComponent(kArray, (state) ? 1 : 0);
    return ret;
}

UpdateDisplayError UpdateDisplaySetMotor(bool state) {
    UpdateDisplayError ret = updateDisplaySetComponent(kMotor, (state) ? 1 : 0);
    return ret;
}

UpdateDisplayError UpdateDisplaySetGear(TriState gear) {
    UpdateDisplayError ret = updateDisplaySetComponent(kGear, (uint32_t)gear);
    if (ret != kUpdateDisplayErrNone) {
        return ret;
    }

    ret = updateDisplayRefresh();
    return ret;
}

UpdateDisplayError UpdateDisplaySetRegenState(TriState state) {
    UpdateDisplayError ret =
        updateDisplaySetComponent(kRegenSt, (uint32_t)state);
    if (ret != kUpdateDisplayErrNone) {
        return ret;
    }

    ret = updateDisplayRefresh();
    return ret;
}

UpdateDisplayError UpdateDisplaySetCruiseState(TriState state) {
    UpdateDisplayError ret =
        updateDisplaySetComponent(kCruiseSt, (uint32_t)state);
    if (ret != kUpdateDisplayErrNone) {
        return ret;
    }

    ret = updateDisplayRefresh();
    return ret;
}

/**
 * @brief Clears the display message queue and sets the message counter
 * semaphore value to 0
 */
void UpdateDisplayClearQueue() {
    OS_ERR err = 0;
    OSSemSet(&display_q_sem4, 0,
             &err);  // Set the message queue semaphore value to 0
    if (err != OS_ERR_TASK_WAITING) {
        ASSERT_OS_ERROR(err);  // Don't fault if UpdateDisplay is waiting
    }
    DispFifoRenew(&msg_queue);  // Clear the message queue
}

/**
 * @brief Loops through the display queue and sends all messages
 */
void TaskUpdateDisplay(void* p_arg) {
    while (1) {
        updateDisplayPopNext();
    }
}

/**
 * Error handler functions
 * Passed as callback functions to the main ThrowTaskError function by
 * assertUpdateDisplayError
 */

/**
 * @brief A handler callback function run by the main ThrowTaskError function
 * used if we haven't reached the restart limit and encounter an error
 */
static void handlerUpdateDisplayRestart() {
    UpdateDisplayClearQueue();  // Clear the message queue
    DisplayReset();             // Try resetting to fix the display error
}

/**
 * @brief Check for a display error and assert it if it exists.
 * Stores the error code, calls the main assertion function
 * and runs a callback function as a handler to restart the display and clear
 * the queue.
 * @param   err variable with display error codes
 */
static void assertUpdateDisplayError(UpdateDisplayError err) {
    error_update_display =
        (ErrorCode)err;  // Store the error code for inspection

    if (err == kUpdateDisplayErrNone) {
        return;  // No error, return
    }

    // Otherwise try resetting the display using the restart callback
    ThrowTaskError(error_update_display, handlerUpdateDisplayRestart,
                   kOptNoLockSched, kOptRecov);

    error_update_display =
        kUpdateDisplayErrNone;  // Clear the error after handling it
}
