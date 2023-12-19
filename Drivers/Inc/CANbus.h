/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANbus.h
 * @brief 
 * 
 * @addtogroup CANbus
 * @{
 */

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
 * \struct CANLUT_T
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages
 * @param size Size of message data in bytes, should not exceed 8
 */
typedef struct {
	bool idxEn: 1;
	unsigned int size: 7;
} CANLUT_T;

/**
 * Standard CAN packet
 * @param ID 	CANId_t value indicating which message we are trying to send
 * @param idx 	If message is part of a sequence of messages (for messages longer than 64 bits), this indicates the index of the message
 * @param data 	message data
*/
typedef struct {
	CANId_t ID; 		
	uint8_t idx; 		
	uint8_t data[8]; 
} CANDATA_t;

//Compatibility macros for deprecated enum
#define CAN_BLOCKING true
#define CAN_NON_BLOCKING false


/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN. CAN2 will not be supported
 * @param   idWhitelist A list of CAN IDs that we want to receive. If NULL, we will receive all messages
 * @param   idWhitelistSize The size of the whitelist
 * @return  ERROR if bus isn't CAN1 or CAN3, SUCCESS otherwise
 */
ErrorStatus CANbus_Init(CAN_t bus, CANId_t* idWhitelist, uint8_t idWhitelistSize);

/**
 * @brief   Transmits up to 8 bytes of data onto the CANbus. Use an IDX message to send more data
 * @param 	CanData 	The data to be transmitted
 * @param 	blocking 	Whether or not this transmission should be a blocking send
 * @param  	bus			The bus to transmit on. This should be either CARCAN or MOTORCAN
 * @return  ERROR if data wasn't sent, SUCCESS otherwise
 */
ErrorStatus CANbus_Send(CANDATA_t CanData,bool blocking, CAN_t bus);

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers
 * @param   data 		where to store the received message
 * @param   blocking 	Whether or not this read should be blocking
 * @param   bus 		The bus to use, either be CARCAN or MOTORCAN
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CANbus_Read(CANDATA_t* data, bool blocking, CAN_t bus);

#endif


/** @} */
