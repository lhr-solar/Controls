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
const CANLUT_T CANLUT[NUM_CAN_IDS] = {
	[BPS_TRIP]						= {NOIDX, DOUBLE}, /**	   BPS_TRIP						   **/
	[BPS_ALL_CLEAR]					= {NOIDX, DOUBLE}, /**     BPS_ALL_CLEAR				   **/
	[BPS_CONTACTOR_STATE]			= {NOIDX, DOUBLE}, /**	   BPS_CONTACTOR_STATE			   **/
	[CHARGE_ENABLE] 				= {NOIDX, DOUBLE}, /**     CHARGE_ENABLE                   **/
	[STATE_OF_CHARGE] 				= {NOIDX, DOUBLE}, /**     STATE_OF_CHARGE                 **/
	[SUPPLEMENTAL_VOLTAGE] 			= {NOIDX, DOUBLE}, /**     SUPPLEMENTAL_VOLTAGE            **/
	[MC_BUS] 						= {NOIDX, DOUBLE}, /**     MC_BUS                          **/
	[VELOCITY] 						= {NOIDX, DOUBLE}, /**     VELOCITY                        **/
	[MC_PHASE_CURRENT] 				= {NOIDX, DOUBLE}, /**     MC_PHASE_CURRENT                **/
	[VOLTAGE_VEC] 					= {NOIDX, DOUBLE}, /**     VOLTAGE_VEC                     **/
	[CURRENT_VEC] 					= {NOIDX, DOUBLE}, /**     CURRENT_VEC                     **/
	[BACKEMF] 						= {NOIDX, DOUBLE}, /**     BACKEMF                         **/
	[TEMPERATURE] 					= {NOIDX, DOUBLE}, /**     TEMPERATURE                     **/
	[ODOMETER_AMPHOURS] 			= {NOIDX, DOUBLE}, /**     ODOMETER_AMPHOURS               **/
	[ARRAY_CONTACTOR_STATE_CHANGE] 	= {NOIDX, BYTE  }, /**     ARRAY_CONTACTOR_STATE_CHANGE    **/
	[MOTOR_DRIVE] 					= {NOIDX, DOUBLE}, /**     MOTOR_DRIVE                     **/
	[MOTOR_POWER]					= {NOIDX, DOUBLE}, /**     MOTOR_POWER                     **/
	[MOTOR_RESET] 					= {NOIDX, DOUBLE}, /**     MOTOR_RESET                     **/
	[MOTOR_STATUS] 					= {NOIDX, DOUBLE}, /**     MOTOR_STATUS                    **/
	[CARDATA] 						= {NOIDX, DOUBLE}, /**     CARDAT			               **/
};

/**
 * @brief Lists of CAN IDs that we want to receive. Used to initialize the CAN filters for CarCAN and MotorCAN.
*/

CANId_t carCANFilterList[NUM_CARCAN_FILTERS] = {
    BPS_TRIP, 
    STATE_OF_CHARGE,
    SUPPLEMENTAL_VOLTAGE,
    CHARGE_ENABLE
};
CANId_t motorCANFilterList[NUM_MOTORCAN_FILTERS] = {};
