#include "common.h"
#include "os_cfg_app.h"
#include "IOState.h"
#include "Tasks.h"
#include "DebugIO.h"
#include "Pedals.h"
#include "Ignition.h"
#include "Dashboard.h"
#include "StatusLeds.h"

#include "ReadCarCAN.h"
#include "Tasks.h"

static void putIOState(void);

#define IO_STATE_DLY_MS 250u 
#define IO_STATE_HEARTBEAT_DELAY_MS 1000u
#define IO_STATE_HEARTBEAT_DELAY IO_STATE_HEARTBEAT_DELAY_MS/IO_STATE_DLY_MS

#define IOSTATE_ERROR_THRESHOLD 3
#define IOSTATE_TRANSITION_THRESHOLD 3

// havent added transition state stuff yet
#define UNSTABLE_IGN_READING(ign) (ign == IGN_ERROR || ign == IGN_TRANSITION)

void putIOState(void){
    CANDATA_t message = {0};

    message.ID = IO_STATE;

    uint8_t brake = Pedals_Read(BRAKE);

    message.data[0] = Pedals_Read(ACCELERATOR);
    message.data[1] = brake;

    uint8_t s = 0;
    s |= SWITCH_BITMAP_BRAKELIGHT((brake >= PEDAL_BRAKELIGHT_THRESHOLD));
    s |= SWITCH_BITMAP_CRUZ_EN(0);
    s |= SWITCH_BITMAP_CRUZ_ST(0);
    s |= SWITCH_BITMAP_REGEN_SW(0);
    Status_Leds_Write(CRUISE_IND_LED, getDashState(DASHBOARD_CRUZ_SET) ? ON : OFF); // Ceremonial (useless)

    switch(getDashState(DASHBOARD_GEAR)) {
        case FWD: 
            s |= SWITCH_BITMAP_FOR_SW(1); 
            break;
        case REV: 
            s |= SWITCH_BITMAP_REV_SW(1); 
            break;
        default: 
            break;
    }

    ignition_state_t ign = Get_Ignition_State();
        
    // If experiencing an error (short or disconnect) for too long, assert a nonrecoverable fault
    if (UNSTABLE_IGN_READING(ign)) {ign = IGN_OFF; assertIOStateError(IOSTATE_ERROR);}
    
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
void assertIOStateError(IOState_error_code_t io_err)
{
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