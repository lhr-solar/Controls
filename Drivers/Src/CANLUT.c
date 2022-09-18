
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
	{NOIDX, DOUBLE, 0x10C}, /**     CHARGE_ENABLE                   **/
	{NOIDX, DOUBLE, 0x106}, /**     STATE_OF_CHARGE                 **/
	{NOIDX, DOUBLE, 0x10B}, /**     SUPPLEMENTAL_VOLTAGE            **/
	{NOIDX, DOUBLE, 0x580}, /**     CAR_STATE                       **/
	{NOIDX, DOUBLE, 0x242}, /**     MC_BUS                          **/
	{NOIDX, DOUBLE, 0x243}, /**     VELOCITY                        **/
	{NOIDX, DOUBLE, 0x244}, /**     MC_PHASE_CURRENT                **/
	{NOIDX, DOUBLE, 0x245}, /**     VOLTAGE_VEC                     **/
	{NOIDX, DOUBLE, 0x246}, /**     CURRENT_VEC                     **/
	{NOIDX, DOUBLE, 0x247}, /**     BACKEMF                         **/
	{NOIDX, DOUBLE, 0x248}, /**     TEMPERATURE                     **/
	{NOIDX, DOUBLE, 0x24E}, /**     ODOMETER_AMPHOURS               **/
	{NOIDX, BYTE  , 0x24F}, /**     ARRAY_CONTACTOR_STATE_CHANGE    **/
	{NOIDX, DOUBLE, 0x221}, /**     MOTOR_DRIVE                     **/
	{NOIDX, DOUBLE, 0x222}, /**     MOTOR_POWER                     **/
	{NOIDX, DOUBLE, 0x223}, /**     MOTOR_RESET                     **/
	{NOIDX, DOUBLE, 0x241}, /**     MOTOR_STATUS                    **/
	{NOIDX, DOUBLE, 0x243}, /**     MOTOR_VELOCITY                  **/
};
