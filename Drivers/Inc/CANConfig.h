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

/**
 * Filter Lists for CarCAN and MotorCAN
*/
#define NUM_CARCAN_FILTERS 7
#define NUM_MOTORCAN_FILTERS 7
extern  CANId_t carCANFilterList[NUM_CARCAN_FILTERS];
extern  CANId_t motorCANFilterList[NUM_MOTORCAN_FILTERS];


/**
 * The lookup table containing the entries for all of our CAN messages. Located in CANLUT.c
 */
extern const CANLUT_T CANLUT[MAX_CAN_ID];
#endif


/* @} */
