#ifndef CAN_CONFIG
#define CAN_CONFIG
#include "CANbus.h"

#define NUM_FILTERS_CARCAN 4
#define NUM_FILTERS_MOTORCAN 0

/**
 * Filter Lists for CarCAN and MotorCAN
*/
extern const CANId_t carCANFilterList[NUM_FILTERS_CARCAN];
extern const CANId_t motorCANFilterList[NUM_FILTERS_MOTORCAN];

/**
 * The lookup table containing the entries for all of our CAN messages. Located in CANLUT.c
 */
extern const CANLUT_T CANLUT[NUM_CAN_IDS];
#endif