#include "FaultState.h"
#include "Display.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"
#include "MotorController.h"
#include "Contactors.h"
#include "Minions.h"
static bool fromThread = false; //whether fault was tripped from thread


typedef enum {  
    A_CNCTR = 0,
    M_CNCTR,
    CTRL_FAULT,
    LEFT_BLINK,
    RIGHT_BLINK,
    Headlight_ON,
    BrakeLight,
    RSVD_LED
} light_t;


static void ArrayMotorKill(void) {
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
    while(1){;} //nonrecoverable
}

static void nonrecoverableFaultHandler(){
    //turn additional lights on to indicate critical error
//    BSP_GPIO_Write_Pin(LIGHTS_PORT, LEFT_BLINK_PIN, OFF); //Cannot change in new hardware
    Display_SetLight(LEFT_BLINK, ON);
//    BSP_GPIO_Write_Pin(LIGHTS_PORT, RIGHT_BLINK_PIN, OFF);//Cannot change in new hardware
    Display_SetLight(RIGHT_BLINK, ON);
    Minion_Write_Output(BRAKELIGHT, false);
    Display_SetLight(CTRL_FAULT,ON); //turn on fault light
    ArrayMotorKill();
}

void EnterFaultState(void) {
    if(FaultBitmap & FAULT_OS){
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_TRITIUM){ //This gets tripped by the ReadTritium thread
        tritium_error_code_t TritiumError = MotorController_getTritiumError(); //get error code to segregate based on fault type
        if(TritiumError & T_DC_BUS_OVERVOLT_ERR){ //DC bus overvoltage
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_HARDWARE_OVER_CURRENT_ERR){ //Tritium signaled too much current
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_SOFTWARE_OVER_CURRENT_ERR){
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_HALL_SENSOR_ERR){ //hall effect error
            // Note: separate tripcnt from T_INIT_FAIL
            static uint8_t tripcnt = 0; //trip counter
            if(tripcnt>3){ //try to restart the motor a few times and then fail out
                nonrecoverableFaultHandler();
            } else {
                tripcnt++;
                //Lights_Set(CTRL_FAULT,OFF);
                Display_SetLight(CTRL_FAULT,OFF);
                MotorController_Restart(); //re-initialize motor
                return;
            }
        }

        if(TritiumError & T_INIT_FAIL){
            // Note: separate tripcnt from T_HALL_SENSOR_ERR
            static uint8_t tripcnt = 0;
            if(tripcnt>5){
                nonrecoverableFaultHandler(); //we've failed to init the motor five times
            } else {
                tripcnt++;
                //Lights_Set(CTRL_FAULT,OFF);
                Display_SetLight(CTRL_FAULT,OFF);
                MotorController_Restart();
                return;
            }
        }
        return;

        /**
         * FAULTS NOT HANDLED :
         * Low Voltage Lockout Error - Not really much we can do if we're not giving the controller enough voltage,
         * and if we miss drive messages as a result, it will shut itself off.
         * Temp error - Motor will throttle itself, and there's nothing we can do additional to cool it down
         */
    }
    else if(FaultBitmap & FAULT_READBPS){ //This has been put in with future development in mind, it is not currently tripped by anything.
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_UNREACH){ //unreachable code
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_DISPLAY){
        // TODO: Send reset command to display ("reset")
        // To be implemented when display driver is complete
    }
    if(fromThread){//no recovering if fault state was entered outside of the fault thread
        return;
    }
    while(1){;} 
}

void Task_FaultState(void *p_arg) {
    OS_ERR err;
    CPU_TS ts;

    FaultBitmap = FAULT_NONE;
    OSErrLocBitmap = OS_NONE_LOC;

    // Block until fault is signaled by an assert
    while(1){
        OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        fromThread = true;
        EnterFaultState();
        fromThread = false;
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}