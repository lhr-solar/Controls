#ifndef CAN_CONFIG
#define CAN_CONFIG
#include "CANbus.h"




/**
 * Filter Lists for CarCAN and MotorCAN
*/
#define NUM_CARCAN_FILTERS 4
#define NUM_MOTORCAN_FILTERS 0
extern  CANId_t carCANFilterList[NUM_CARCAN_FILTERS];
extern  CANId_t motorCANFilterList[NUM_MOTORCAN_FILTERS];


/**
 * The lookup table containing the entries for all of our CAN messages. Located in CANLUT.c
 */
extern const CANLUT_T CANLUT[MAX_CAN_ID];
#endif