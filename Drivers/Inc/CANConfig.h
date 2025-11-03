/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANConfig.h
 * @brief 
 * 
 * @defgroup CANConfig
 * @addtogroup CANConfig
 * @{
 */
#ifndef CAN_CONFIG
#define CAN_CONFIG
#include "CANbus.h"

#define CARCAN_FILTER_IDS \
    BPS_TRIP, \
    BPS_CONTACTOR, \
    STATE_OF_CHARGE, \
    SUPPLEMENTAL_VOLTAGE, \
    VOLTAGE_SUMMARY, \
    TEMPERATURE_SUMMARY, \
    CURRENT_DATA, \
    CONTACTOR_SENSE, \
    PRECHARGE_TIMEOUT, \
    BPS_FAULT_STATE

#define MOTORCAN_FILTER_IDS \
    MOTOR_STATUS, \
    MC_BUS, \
    VELOCITY, \
    MC_PHASE_CURRENT, \
    BACKEMF, \
    TEMPERATURE, \
    DSP_TEMP, \
    ODOMETER_AMPHOURS, \
    SLIP_SPEED

/**
 * Filter Lists for CarCAN and MotorCAN
*/
#define NUM_CARCAN_FILTERS   (sizeof((CANId_t[]){CARCAN_FILTER_IDS}) / sizeof(CANId_t))
#define NUM_MOTORCAN_FILTERS (sizeof((CANId_t[]){MOTORCAN_FILTER_IDS}) / sizeof(CANId_t))
extern CANId_t carCANFilterList[NUM_CARCAN_FILTERS];
extern CANId_t motorCANFilterList[NUM_MOTORCAN_FILTERS];


/**
 * The lookup table containing the entries for all of our CAN messages. Located in CANLUT.c
 */
extern const CANLUT_T CANLUT[MAX_CAN_ID];
#endif


/* @} */
