/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANConfig.c
 * @brief 
 * 
 */
#include "CANConfig.h"

#define BYTE 1
#define HALFWORD 2
#define WORD 4
#define DOUBLE 8
#define NOIDX false
#define IDX true

/**
 * @brief Lookup table to simplify user-defined packet structs. Contains metadata fields that are always the same for every message of a given ID.
 *        Indexed by CANId_t values. Any changes or additions must be made in parallel with changes made to the CANID_t enum in CANbus.h
 */
const CANLUT_T CANLUT[MAX_CAN_ID] = {
	[BPS_TRIP]						= {NOIDX, DOUBLE}, /**	   BPS_TRIP						   **/
	[BPS_CONTACTOR]		        	= {NOIDX, DOUBLE}, /**	   BPS_CONTACTOR		           **/
	[STATE_OF_CHARGE] 				= {NOIDX, DOUBLE}, /**     STATE_OF_CHARGE                 **/
	[SUPPLEMENTAL_VOLTAGE] 			= {NOIDX, DOUBLE}, /**     SUPPLEMENTAL_VOLTAGE            **/
	[MOTOR_DRIVE] 					= {NOIDX, DOUBLE}, /**     MOTOR_DRIVE                     **/
	[MOTOR_POWER]					= {NOIDX, DOUBLE}, /**     MOTOR_POWER                     **/
	[MOTOR_RESET] 					= {NOIDX, DOUBLE}, /**     MOTOR_RESET                     **/
	[MOTOR_STATUS] 					= {NOIDX, DOUBLE}, /**     MOTOR_STATUS                    **/
	[MC_BUS] 						= {NOIDX, DOUBLE}, /**     MC_BUS                          **/
	[VELOCITY] 						= {NOIDX, DOUBLE}, /**     VELOCITY                        **/
	[MC_PHASE_CURRENT] 				= {NOIDX, DOUBLE}, /**     MC_PHASE_CURRENT                **/
	[VOLTAGE_VEC] 					= {NOIDX, DOUBLE}, /**     VOLTAGE_VEC                     **/
	[CURRENT_VEC] 					= {NOIDX, DOUBLE}, /**     CURRENT_VEC                     **/
	[BACKEMF] 						= {NOIDX, DOUBLE}, /**     BACKEMF                         **/
	[TEMPERATURE] 					= {NOIDX, DOUBLE}, /**     TEMPERATURE                     **/
	[ODOMETER_AMPHOURS] 			= {NOIDX, DOUBLE}, /**     ODOMETER_AMPHOURS               **/
	[ARRAY_CONTACTOR_STATE_CHANGE] 	= {NOIDX, BYTE  }, /**     ARRAY_CONTACTOR_STATE_CHANGE    **/
    [SLIP_SPEED]                    = {NOIDX, DOUBLE},
	[MOTOR_DRIVE] 					= {NOIDX, DOUBLE}, /**     MOTOR_DRIVE                     **/
	[MOTOR_POWER]					= {NOIDX, DOUBLE}, /**     MOTOR_POWER                     **/
	[MOTOR_RESET] 					= {NOIDX, DOUBLE}, /**     MOTOR_RESET                     **/
	[MOTOR_STATUS] 					= {NOIDX, DOUBLE}, /**     MOTOR_STATUS                    **/
	[IO_STATE] 				        = {NOIDX, DOUBLE}, /**     IO_STATE			               **/
    [CONTROL_MODE]                  = {NOIDX, BYTE  }, /**     CONTROL_MODE			           **/
};

/**
 * @brief Lists of CAN IDs that we want to receive. Used to initialize the CAN filters for CarCAN and MotorCAN.
*/

CANId_t carCANFilterList[NUM_CARCAN_FILTERS] = {
    BPS_TRIP, 
	BPS_CONTACTOR,			// Bit 1 and 0 contain BPS HV Plus/Minus (associated Motor Controller) Contactor and BPS HV Array Contactor, respectively
    STATE_OF_CHARGE,	
    SUPPLEMENTAL_VOLTAGE
};

CANId_t motorCANFilterList[NUM_MOTORCAN_FILTERS] = {
    MC_BUS,
    VELOCITY,
    BACKEMF,
    TEMPERATURE,
    ODOMETER_AMPHOURS,
    SLIP_SPEED,
    MOTOR_STATUS
};
