/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef CAN_H__
#define CAN_H__

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
 * (the enum is sorted in ascending order on purpus), and then add an entry to the lookup table.
 */
typedef enum { 
	CHARGE_ENABLE 					= 0x10C,
	STATE_OF_CHARGE 				= 0x106,
	SUPPLEMENTAL_VOLTAGE 			= 0x10B,
	// CAR_STATE 						= 0x580,
	MOTOR_DRIVE 					= 0x221,
	MOTOR_POWER						= 0x222,
	MOTOR_RESET 					= 0x223,
	MOTOR_STATUS 					= 0x241,
	MOTOR_VELOCITY 					= 0x243,
	MC_BUS 							= 0x242,
	VELOCITY 						= 0x243,
	MC_PHASE_CURRENT 				= 0x244,
	VOLTAGE_VEC 					= 0x245,
	CURRENT_VEC 					= 0x246,
	BACKEMF 						= 0x247,
	TEMPERATURE 					= 0x248,
	ODOMETER_AMPHOURS 				= 0x24E,
	ARRAY_CONTACTOR_STATE_CHANGE 	= 0x24F,
	NUM_CAN_IDS
} CANId_t;

/**
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages.
 * @param size Size of message's data
 * @param ID The actual CAN ID of this message
 */
typedef struct {
	bool idxEn;
	uint8_t size;
} CANLUT_T;

/**
 * The lookup table containing the entries for all of our CAN messages. Located in CANLUT.c
 */
extern CANLUT_T CANLUT[NUM_CAN_IDS];

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

/**
 * Standard identifier for whether or not a CAN transaction is blocking or not
 */
typedef enum {CAN_BLOCKING=0, CAN_NON_BLOCKING} CAN_blocking_t;

/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN
 * @return  None
 */
void CANbus_Init(CAN_t bus);

/**
 * @brief   Transmits data onto the CANbus. Transmits up to 8 bytes at a time. If more is needed, 
 * @param 	CanData 	The data to be transmitted
 * @param 	blocking 	Whether or not this transmission should be a blocking send.
 * @param  	bus			The bus to transmit on. This should be either CARCAN or MOTORCAN.
 * @return  ERROR if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANDATA_t CanData,CAN_blocking_t blocking, CAN_t bus);

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers.
 * @param   data 		pointer to where to store the CAN id of the recieved msg
 * @param   blocking 	Whether or not this read should be a blocking read
 * @param   bus 		The bus to use. This should either be CARCAN or MOTORCAN.
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CANbus_Read(CANDATA_t* data, CAN_blocking_t blocking, CAN_t bus);

#endif
