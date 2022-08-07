/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef CAN_H__
#define CAN_H__

#include "BSP_CAN.h"
#define CARCAN CAN_1 //convenience aliases for the CANBuses
#define MOTORCAN CAN_3
#define MAX_CAN_LEN 8
typedef enum {
	MC_BUS = 0x242,
	VELOCITY = 0x243,
	MC_PHASE_CURRENT = 0x244,
	VOLTAGE_VEC = 0x245,
	CURRENT_VEC = 0x246,
	BACKEMF = 0x247,
	TEMPERATURE = 0x24B,
	ODOMETER_AMPHOURS = 0x24E,
	ARRAY_CONTACTOR_STATE_CHANGE = 0x24F,
	CAR_STATE = 0x580,
	CHARGE_ENABLE = 0x10C,
	SUPPLEMENTAL_VOLTAGE = 0x10B,
	STATE_OF_CHARGE = 0x106,
	MOTOR_DRIVE = 0x221,
	MOTOR_POWER = 0x222,
	MOTOR_RESET = 0x223,
	MOTOR_STATUS = 0x241,
	MOTOR_VELOCITY = 0x243,
	DO_NOT_USE_FORCE_ENUM_SIZE = 0xFFFFFFFF //force the enum values to be 32 bits wide. This value should not be used. This is here because the BSP layer expects a 32 bit ptr for ID's
} CANId_t;




/**
 * Standard CAN packet
*/
typedef struct {
	CANId_t ID; 		//ID of message
	uint8_t idx; 		//FOR TRANSMIT ONLY: if message is part of a sequence of messages (for messages longer than 64 bits), this indicates the index of the message. Recieve will not touch this
	bool idxEn; 		//FOR TRANSMIT ONLY: whether to use idx or not. Recieve will not touch this.
	uint8_t size; 		//size of this particular message IN BYTES. On writes, this should not Exceed 8 bytes for non-idx messages, and should not exceed 7 for idx messages.
	uint64_t data; 	//data of the message
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
