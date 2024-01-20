/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_READTRITIUM
#include_next "ReadTritium.h"
#else

#ifndef __READ_TRITIUM_H
#define __READ_TRITIUM_H

#include "fff.h"
#include "os.h"
#include "common.h"
#include "Tasks.h"

/**
 * Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate error messages as needed based on bits
 * 
 */
typedef enum{
    T_HARDWARE_OVER_CURRENT_ERR = (1<<0), 
    T_SOFTWARE_OVER_CURRENT_ERR = (1<<1), 
    T_DC_BUS_OVERVOLT_ERR = (1<<2), 
    T_HALL_SENSOR_ERR = (1<<3), 
    T_WATCHDOG_LAST_RESET_ERR = (1<<4), 
    T_CONFIG_READ_ERR = (1<<5), 
    T_UNDER_VOLTAGE_LOCKOUT_ERR = (1<<6), 
    T_DESAT_FAULT_ERR = (1<<7), 
    T_MOTOR_OVER_SPEED_ERR = (1<<8), 
    T_INIT_FAIL = (1<<9), //motor controller fails to restart or initialize
    T_MOTOR_WATCHDOG_TRIP = (1 << 15),
    T_NONE = 0x00,
} tritium_error_code_t;

DECLARE_FAKE_VOID_FUNC(Task_ReadTritium, void*);

DECLARE_FAKE_VALUE_FUNC(float, Motor_RPM_Get);

DECLARE_FAKE_VALUE_FUNC(float, Motor_Velocity_Get);

#endif
#endif
