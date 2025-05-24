#include "common.h"
#include "os_cfg_app.h"

#include "DebugIO.h"
#include "Pedals.h"
#include "Ignition.h"
#include "Contactors.h"
#include "Dashboard.h"
#include "StatusLeds.h"
#include "SendTritium.h"
#include "CANbus.h"
#include "Tasks.h"
#include "IOState.h"

static void putIOState(void);

#define IO_STATE_DLY_MS 250u 
#define IO_STATE_HEARTBEAT_DELAY_MS 1000u
#define IO_STATE_HEARTBEAT_DELAY IO_STATE_HEARTBEAT_DELAY_MS/IO_STATE_DLY_MS

#define IOSTATE_ERROR_THRESHOLD 4
#define IOSTATE_TRANSITION_THRESHOLD 8

// havent added transition state stuff yet
#define UNSTABLE_IGN_READING(ign) (ign == IGN_ERROR || ign == IGN_TRANSITION)

void putIOState(void){
    CANDATA_t message = {0};

    message.ID = IO_STATE;

    uint8_t brake = Pedals_Read(BRAKE);

    message.data[0] = Pedals_Read(ACCELERATOR);
    message.data[1] = brake;

    uint8_t s = 0;
    s |= SWITCH_BITMAP_BRAKELIGHT(get_isBrakeOn());
    s |= SWITCH_BITMAP_CRUZ_EN(0);
    s |= SWITCH_BITMAP_CRUZ_ST(0);
    s |= SWITCH_BITMAP_REGEN_SW(0);
    Status_Leds_Write(CRUISE_IND_LED, getSwitchState(DASH_CRUZ_SET) ? ON : OFF); // Ceremonial (useless)

    switch(getGear()) {
        case DASH_FWD: 
            s |= SWITCH_BITMAP_FOR_SW(1); 
            break;
        case DASH_REV: 
            s |= SWITCH_BITMAP_REV_SW(1); 
            break;
        default: 
            break;
    }

    static ignition_state_t prev_state = IGN_OFF; // Return last state when switching between positions
    static uint8_t transition_count = 0; // Assert recoverable error and set to off if in transition too long
    static uint8_t error_count = 0; // Assert nonrecoverable error if there are too many ignition errors

    ignition_state_t ign = Get_Ignition_State();

    if (UNSTABLE_IGN_READING(ign)) {
        // Update ignition counters
        if (ign == IGN_ERROR) error_count++;
        if (ign == IGN_TRANSITION) transition_count++;
        ign = prev_state; // Return last state
    } 
    else { // Valid ignition state
        prev_state = ign;
        transition_count = 0;
        error_count  = 0;
    }
    
    // If in an unstable state for too long, return IGN_OFF and fault
    if (transition_count > IOSTATE_TRANSITION_THRESHOLD || error_count > IOSTATE_ERROR_THRESHOLD) {
        ign = IGN_OFF;
        assertIOStateError(IOSTATE_ERROR);
    }
    
    if (ign >= IGN_ARR) s |= SWITCH_BITMAP_IGN_1_ARRAY(1);
    if (ign == IGN_MOTOR) s |= SWITCH_BITMAP_IGN_2_MOTOR(1);
    
    message.data[2] = s;

    CANbus_Send(message, true, CARCAN);

    // // Update the global event flag group
    // OS_ERR err;
    // CPU_TS ticks;
    // OS_FLAGS res_set = OSFlagPend(&BPS_Motor_Status_Flags, BPS_SAFE | BPS_CHECKED, 0, OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_NON_BLOCKING, &ticks, &err);
    // assertOSError(err);

    // // IF BPS Safe & in motor ignition rotary switch position & motor controller precharge bypass contactor is closed, mark motor ready to run
    // // NOTE: BPS Safe means HV+ & HV- are closed
    // if(res_set && res_clr && (Get_Ignition_State() == IGN_MOTOR) && Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true) && Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true)) {
    //     OSFlagPost(&BPS_Motor_Status_Flags, MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_SET, &err);
    //     assertOSError(err);
    // }
    // else {
    //     OSFlagPost(&BPS_Motor_Status_Flags, MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_CLR, &err);
    //     assertOSError(err);
    // }
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
        if(ioStateCounter >= IO_STATE_HEARTBEAT_DELAY){
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
 * Stores the error code and calls assertTaskError with the appropriate parameters and callback handler
 * @param  io_err error code to specify the issue encountered
 */
void assertIOStateError(IOState_error_code_t io_err) {
    Error_IOState = (error_code_t)io_err; // Store error code for inspection
    set_errmsg_hex("IOS", ErrMsg_IOState, io_err);    // Store error message for inspection
    
    switch (io_err) {
        case IOSTATE_ERR_NONE:
            break;

        case IOSTATE_ERROR: // More than one state on for multiple cycles. Set IGN to OFF and fault
            strncpy(ErrMsg_Evac, DISP_EVACMSG_DEFAULT, ERR_CODE_LEN);
            throwTaskError(Error_IOState, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;
        
        default:
            break;
    }

    Error_IOState = IOSTATE_ERR_NONE; // Clear the error after handling it
}