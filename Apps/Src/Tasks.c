/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Tasks.h"
#include "FaultState.h"
#include "os.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"


/**
 * TCBs
 */
OS_TCB FaultState_TCB;
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB Telemetry_TCB;

/**
 * Stacks
 */
CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE];
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK Telemetry_Stk[TASK_TELEMETRY_STACK_SIZE];


/**
 * Global Variables
 */
//fault_bitmap_t FaultBitmap = FAULT_NONE;
//os_error_loc_t OSErrLocBitmap = OS_NONE_LOC;
extern const PinInfo_t PINS_LOOKARR[]; // For GPIO writes. Externed from Minions Driver C file.

void _assertOSError(uint16_t OS_err_loc, OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);
        
        BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
        BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
        while(1){;} //nonrecoverable

    }
}

