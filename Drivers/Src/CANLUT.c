
#include "CANbus.h"

#define BYTE 1
#define HALFWORD 2
#define WORD 4
#define DOUBLE 8
#define NOIDX false
#define IDX true


/**
 * @brief Lookup table to simplify user-defined packet structs. Contains fields that are always the same for every message of a given ID.
 *        Indexed by CANId_t values. Any changes or additions must be made in parallel with changes made to the CANID_t enum in CANbus.h
 */
CANLUT_T CANLUT[NUM_CAN_IDS] = {
	[CHARGE_ENABLE] 				= {NOIDX, DOUBLE}, /**     CHARGE_ENABLE                   **/
	[STATE_OF_CHARGE] 				= {NOIDX, DOUBLE}, /**     STATE_OF_CHARGE                 **/
	[SUPPLEMENTAL_VOLTAGE] 			= {NOIDX, DOUBLE}, /**     SUPPLEMENTAL_VOLTAGE            **/
	// [CAR_STATE] = {NOIDX, DOUBLE, 0x580}, /**     CAR_STATE                       **/
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
	[MOTOR_VELOCITY] 				= {NOIDX, DOUBLE}, /**     MOTOR_VELOCITY                  **/
};
