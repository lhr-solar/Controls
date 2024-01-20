/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_CANBUS
#include_next "CANbus.h"
#else

#ifndef CAN_H__
#define CAN_H__

#include "fff.h"
#include "BSP_CAN.h"


#define CARCAN CAN_1 //convenience aliases for the CANBuses
#define MOTORCAN CAN_3

/**
 * This enum is used to signify the ID of the message you want to send. 
 * It is used internally to index our lookup table (CANLUT.C) and get message-specific fields.
 * For user purposes, it selects the message to send.
 * 
 * If changing the order of this enum, make sure to mirror that change in the lookup table, or
 * else the driver will not work properly. 
 * 
 * If adding new types of CAN messages, add the identifier wherever it fits in 
 * (the enum is sorted in ascending order on purpose), and then add an entry to the lookup table.
 */
typedef enum { 
	BPS_TRIP						= 0x002,
	BPS_CONTACTOR					= 0x101,
	STATE_OF_CHARGE 				= 0x106,
	SUPPLEMENTAL_VOLTAGE 			= 0x10B,
	MOTOR_DRIVE 					= 0x221,
	MOTOR_POWER						= 0x222,
	MOTOR_RESET 					= 0x223,
	MOTOR_STATUS 					= 0x241,
	MC_BUS 							= 0x242,
	VELOCITY 						= 0x243,
	MC_PHASE_CURRENT 				= 0x244,
	VOLTAGE_VEC 					= 0x245,
	CURRENT_VEC 					= 0x246,
	BACKEMF 						= 0x247,
	TEMPERATURE 					= 0x24B,
	ODOMETER_AMPHOURS 				= 0x24E,
	ARRAY_CONTACTOR_STATE_CHANGE 	= 0x24F,
	CONTROL_MODE                    = 0x580,
    IO_STATE 						= 0x581,
	MAX_CAN_ID
} CANId_t;

/**
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages.
 * @param size Size of message's data. Should be a maximum of eight (in decimal).
 */
typedef struct {
	bool idxEn: 1;
	unsigned int size: 7;
} CANLUT_T;

/**
 * Standard CAN packet
 * @param ID 	CANId_t value indicating which message we are trying to send
 * @param idx 	If message is part of a sequence of messages (for messages longer than 64 bits), this indicates the index of the message. 
 * 				This is not designed to exceed the 8bit unsigned max value.
 * @param data 	data of the message
*/
typedef struct {
	CANId_t ID; 		
	uint8_t idx; 		
	uint8_t data[8]; 
} CANDATA_t;

//Compatibility macros for deprecated enum
#define CAN_BLOCKING true
#define CAN_NON_BLOCKING false

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Init, CAN_t, CANId_t*, uint8_t);

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Send, CANDATA_t, bool, CAN_t);

DECLARE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Read, CANDATA_t*, bool, CAN_t);

#endif
#endif