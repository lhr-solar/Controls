#include "common.h"
#include "os_cfg_app.h"
#include "IOState.h"
#include "Tasks.h"
#include "Pedals.h"
#include "Ignition.h"
#include "Dashboard.h"
#include "StatusLeds.h"

static void putIOState(void);

#define IO_STATE_DLY_MS 250u 
#define IO_STATE_HEARTBEAT_DELAY (1000u)/IO_STATE_DLY_MS

void putIOState(void){
    CANDATA_t message;
    memset(&message, 0, sizeof message);
    message.ID = IO_STATE;

    // Get pedal information
    message.data[0] = Pedals_Read(ACCELERATOR);
    int8_t brake_pedal = Pedals_Read(BRAKE);
    message.data[1] = brake_pedal;

    // If the brake is pressed far enough, send the state to turn on the brakelight
    message.data[2] |= SWITCH_BITMAP_BRAKELIGHT((brake_pedal >= PEDAL_BRAKELIGHT_THRESHOLD) ? 1:0);

    // Send Cruise states
    message.data[2] |= SWITCH_BITMAP_CRUZ_EN(getDashState(DASHBOARD_CRUZ_EN));
    message.data[2] |= SWITCH_BITMAP_CRUZ_ST(getDashState(DASHBOARD_CRUZ_SET));

    // Set Cruise Indicator LED
    Status_Leds_Write(CRUISE_IND_LED, getDashState(DASHBOARD_CRUZ_SET) ? ON : OFF);

    // Regen is always disabled for daybreak
    message.data[2] |= SWITCH_BITMAP_REGEN_SW(0);

    switch(getDashState(DASHBOARD_GEAR)){
        case FWD:
            message.data[2] |= SWITCH_BITMAP_FOR_SW(1);
            break;
        case REV:
            message.data[2] |= SWITCH_BITMAP_REV_SW(1);
            break;
        default:
            message.data[2] |= SWITCH_BITMAP_FOR_SW(0);
            message.data[2] |= SWITCH_BITMAP_REV_SW(0);
            break;
    }

    // Send ignition states
    switch(Get_Ignition_State()){
        case IGN_ARR:
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(1);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(0);
            break;
        case IGN_MOTOR:
            // motor comes after array state, so both are set
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(1);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(1);
            break;
        default:
            message.data[2] |= SWITCH_BITMAP_IGN_1_ARRAY(0);
            message.data[2] |= SWITCH_BITMAP_IGN_2_MOTOR(0);
            break;
    }

    CANbus_Send(message, true, CARCAN);
}

/**
 * @brief sends IO information over CarCAN every IO_STATE_DLY_MS
*/
void Task_IOState(void *p_arg) {
    OS_ERR err;
    static uint8_t ioStateCounter = 0;
    while (1) {
        putIOState();
        ioStateCounter++;
        // toggle dashboard led every 1 second (IoState runs at 250ms)
        if(ioStateCounter >= IO_STATE_HEARTBEAT_DELAY){
            Status_Leds_Toggle(DASH_HEARTBEAT_LED); // heartbeat led on the dashboard
            ioStateCounter = 0;
        }
        OSTimeDlyHMSM(0, 0, 0, IO_STATE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }  
}
