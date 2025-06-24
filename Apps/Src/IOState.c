#include "os_cfg_app.h"

#include "CANbus.h"
#include "Contactors.h"
#include "Dashboard.h"
#include "DebugIO.h"
#include "Ignition.h"
#include "Pedals.h"
#include "StatusLeds.h"
#include "Lights.h"

#include "IOState.h"
#include "SendTritium.h"
#include "Tasks.h"

#define IO_STATE_DLY_MS              250u
#define IO_STATE_HEARTBEAT_DELAY_MS  1000u
#define IO_STATE_HEARTBEAT_DELAY     IO_STATE_HEARTBEAT_DELAY_MS / IO_STATE_DLY_MS

#define IOSTATE_ERROR_THRESHOLD      4
#define IOSTATE_TRANSITION_THRESHOLD 8

// havent added transition state stuff yet
#define UNSTABLE_IGN_READING(ign)    (ign == IGN_ERROR || ign == IGN_TRANSITION)

void putIOState(void) {
    CANDATA_t message = {0};

    message.ID = IO_STATE;

    message.data[0] = Pedals_Read(ACCELERATOR);
    message.data[1] = Pedals_Read(BRAKE); // raw analog value

    uint8_t s = 0;
    s |= SWITCH_BITMAP_BRAKELIGHT(get_isBrakeOn());
    s |= SWITCH_BITMAP_CRUZ_EN(0);
    s |= SWITCH_BITMAP_CRUZ_ST(0);
    s |= SWITCH_BITMAP_REGEN_SW(0);
    Status_Leds_Write(CRUISE_IND_LED, getSwitchState(DASH_CRUZ_SET) ? ON : OFF);

    Lights_Write(RIGHT_LIGHT, getSwitchState(DASH_RIGHT_SIG)? 50 : 0);
    Lights_Write(LEFT_LIGHT, getSwitchState(DASH_LEFT_SIG)? 10 : 0);

    switch (getGear(GEAR_USE_OS_DELAY)) {
        case DASH_FWD:
            s |= SWITCH_BITMAP_FOR_SW(1);
            break;
        case DASH_REV:
            s |= SWITCH_BITMAP_REV_SW(1);
            break;
        default:
            break;
    }

    // Return last state when switching between positions
    static ignition_state_t prev_state = IGN_OFF;
    // Assert recoverable error and set to off if in transition too long
    static uint8_t transition_count = 0;
    // Assert nonrecoverable error if there are too many ignition errors
    static uint8_t error_count = 0;

    ignition_state_t ign = Get_Ignition_State();

    if (UNSTABLE_IGN_READING(ign)) {
        // Update ignition counters
        if (ign == IGN_ERROR) error_count++;
        if (ign == IGN_TRANSITION) transition_count++;
        ign = prev_state; // Return last state
    } else {              // Valid ignition state
        prev_state = ign;
        transition_count = 0;
        error_count = 0;
    }

    // If in an unstable state for too long, return IGN_OFF and fault
    if (transition_count > IOSTATE_TRANSITION_THRESHOLD || error_count > IOSTATE_ERROR_THRESHOLD) {
        ign = IGN_OFF;
        assertIOStateError(C_ERR_IOS_IGN_FAULT);
    }

    if (ign >= IGN_ARR) s |= SWITCH_BITMAP_IGN_1_ARRAY(1);
    if (ign == IGN_MOTOR) s |= SWITCH_BITMAP_IGN_2_MOTOR(1);

    message.data[2] = s;

    CANbus_Send(message, true, CARCAN);
}

/**
 * @brief sends IO information over CarCAN every IO_STATE_DLY_MS
 */
void Task_IOState(void *p_arg) {
    OS_ERR err;
    static volatile uint8_t ioStateCounter = 0;
    while (1) {
#ifdef TASK_PROFILER
        DebugIO_Toggle(IO_STATE_PIN);
#endif

        putIOState();
        ioStateCounter++;
        // toggle dashboard led every 1 second (IoState runs at 250ms)
        if (ioStateCounter >= IO_STATE_HEARTBEAT_DELAY) {
            Status_Leds_Toggle(DASH_HEARTBEAT_LED); // heartbeat led on the dashboard
            ioStateCounter = 0;
        }

#ifdef TASK_PROFILER
        DebugIO_Toggle(IO_STATE_PIN);
#endif
        OSTimeDlyHMSM(0, 0, 0, IO_STATE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
}

/**
 * @brief error assertion function for IOState, used to handle ignition errors
 * Stores the error code and calls assertTaskError with the appropriate parameters and callback
 * handler
 * @param  io_err error code to specify the issue encountered
 */
void assertIOStateError(controls_error_e io_err) {
    switch (io_err) {
        case C_ERR_NONE:
            break;

        case C_ERR_IOS_GENERIC:
        case C_ERR_IOS_IGN_FAULT: // Reading != 1 ign state too many times. Set IGN to OFF and fault
            throwTaskError(io_err, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;

        default:
            // Critical failure, we have a non sendtritium error in send tritium somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;
    }
}