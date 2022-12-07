#include "FaultState.h"
#include "Display.h"
#include "Contactors.h"
#include "os.h"
#include "Tasks.h"
#include "Contactors.h"
#include "Minions.h"
#include "ReadTritium.h"

#define RESTART_THRESHOLD 3

static bool fromThread = false; //whether fault was tripped from thread
extern const PinInfo_t PINS_LOOKARR[]; // For GPIO writes. Externed from Minions Driver C file.


static void ArrayMotorKill(void) {
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
    while(1){;} //nonrecoverable
}

static void nonrecoverableFaultHandler(){
    //turn additional brakelight on to indicate critical error
    BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);
    ArrayMotorKill();
}

void EnterFaultState(void) {
    if(FaultBitmap & FAULT_OS){
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_TRITIUM){ //This gets tripped by the ReadTritium thread
        tritium_error_code_t TritiumError = MotorController_getTritiumError(); //get error code to segregate based on fault type
    
        if(TritiumError & T_HARDWARE_OVER_CURRENT_ERR){ //Tritium signaled too much current
            nonrecoverableFaultHandler();
        }
        
        if(TritiumError & T_SOFTWARE_OVER_CURRENT_ERR){
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_DC_BUS_OVERVOLT_ERR){ //DC bus overvoltage
            nonrecoverableFaultHandler();
        }
        
        if(TritiumError & T_HALL_SENSOR_ERR){ //hall effect error
            // Note: separate tripcnt from T_INIT_FAIL
            static uint8_t hall_fault_cnt = 0; //trip counter
            if(hall_fault_cnt > RESTART_THRESHOLD){ //try to restart the motor a few times and then fail out
                nonrecoverableFaultHandler();
            } else {
                hall_fault_cnt++;
                MotorController_Restart(); //re-initialize motor
                return;
            }
        }

        if(TritiumError & T_CONFIG_READ_ERR){ //Config read error
            nonrecoverableFaultHandler();
        }
            
        if(TritiumError & T_DESAT_FAULT_ERR){ //Desaturation fault error
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_MOTOR_OVER_SPEED_ERR){ //Motor over speed error
            nonrecoverableFaultHandler();
        }

        if(TritiumError & T_INIT_FAIL){ //motorcontroller fails to restart or initialize
            nonrecoverableFaultHandler();
        }

        return;

        /**
         * FAULTS NOT HANDLED :
         * Watchdog Last Reset Error - Not using any hardware watchdogs. 
         * 
         * Under Voltage Lockout Error - Not really much we can do if we're not giving the controller enough voltage,
         * and if we miss drive messages as a result, it will shut itself off.

         */
    }
    else if(FaultBitmap & FAULT_READBPS){ //This has been put in with future development in mind, it is not currently tripped by anything.
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_UNREACH){ //unreachable code
        nonrecoverableFaultHandler();
    }
    else if(FaultBitmap & FAULT_DISPLAY){
        static uint8_t disp_fault_cnt = 0;
        if(disp_fault_cnt>3){
            Display_Fault(OSErrLocBitmap, FaultBitmap);
        } else {
            disp_fault_cnt++;
            Display_Reset();
            return;
        }
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