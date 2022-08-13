/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef CAN_H__
#define CAN_H__

#include "BSP_CAN.h"
#define CARCAN CAN_1 //convenience aliases for the CANBuses
#define MOTORCAN CAN_3
#define MAX_CAN_LEN 8


/**
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages.
 * @param size Size of message's data
 * @param ID The actual CAN ID of this message
 */
typedef struct {
	bool idxEn;
	uint8_t size;
	uint32_t ID;
} CANLUT_T;

/**
 * @brief Lookup table to simplify user-defined packet structs. Contains fields that are always the same for every message of a given ID.
 * Indexed by CANId_t values.
 * FOR DRIVER USE ONLY.
 */
static CANLUT_T CANLUT[NUM_CAN_IDS] = {
	{false, MAX_CAN_LEN, 0x10C},
	{false, MAX_CAN_LEN, 0x106},
	{false, MAX_CAN_LEN, 0x10B},
	{false, MAX_CAN_LEN, 0x580},
	{false, MAX_CAN_LEN, 0x242},
	{false, MAX_CAN_LEN, 0x243},
	{false, MAX_CAN_LEN, 0x244},
	{false, MAX_CAN_LEN, 0x245},
	{false, MAX_CAN_LEN, 0x246},
	{false, MAX_CAN_LEN, 0x247},
	{false, MAX_CAN_LEN, 0x248},
	{false, MAX_CAN_LEN, 0x24E},
	{false, MAX_CAN_LEN, 0x24F},
	{false, MAX_CAN_LEN, 0x221},
	{false, MAX_CAN_LEN, 0x222},
	{false, MAX_CAN_LEN, 0x223},
	{false, MAX_CAN_LEN, 0x241},
	{false, MAX_CAN_LEN, 0x243},
};

/**
 * @brief This enum is used to signify the ID of the message you want to send. 
 * It is used internally to index our lookup table (above) and get message-specific fields.
 * If changing the order of this enum, make sure to mirror that change in the lookup table, or
 * else the driver will not work properly.
 */
typedef enum { 
	CHARGE_ENABLE = 0,
	STATE_OF_CHARGE,
	SUPPLEMENTAL_VOLTAGE,
	CAR_STATE,
	MC_BUS,
	VELOCITY,
	MC_PHASE_CURRENT,
	VOLTAGE_VEC,
	CURRENT_VEC,
	BACKEMF,
	TEMPERATURE,
	ODOMETER_AMPHOURS,
	ARRAY_CONTACTOR_STATE_CHANGE,
	MOTOR_DRIVE,
	MOTOR_POWER,
	MOTOR_RESET,
	MOTOR_STATUS,
	MOTOR_VELOCITY,
	NUM_CAN_IDS
} CANId_t;


/**
 * Standard CAN packet
 * @param ID CANId_t value indicating which message we are trying to send
 * @param idx FOR TRANSMIT ONLY: 
 * if message is part of a sequence of messages (for messages longer than 64 bits), this indicates the index of the message. 
 * This is not designed to exceed the 8bit unsigned max value.
 * @param data data of the message
*/
typedef struct {
	CANId_t ID; 		
	uint8_t idx; 		
	uint8_t data[8]; 
} CANDATA_t;


typedef enum {CAN_BLOCKING=0, CAN_NON_BLOCKING} CAN_blocking_t;

/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN
 * @return  None
 */
void CANbus_Init(CAN_t bus);

/**
 * @brief   Transmits data onto the CANbus. Transmits up to 8 bytes at a time. If more is needed, 
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param   blocking: Whether or not the Send should be blocking or not
 * @param   bus: Which bus to transmit on
 * @return  ERROR if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANDATA_t CanData,CAN_blocking_t blocking, CAN_t bus);

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers. 
 * 	DOES NOT POPULATE IDXen or IDX. You have to manually inspect the first byte and the ID
 * @param   ID pointer to where to store the CAN id of the recieved msg
 * @param   pointer to buffer array to store message. MUST BE 8 BYTES OR LARGER
 * @param   blocking whether or not this read should be blocking
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CANbus_Read(CANDATA_t* data, CAN_blocking_t blocking, CAN_t bus);

#endif
