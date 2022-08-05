/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef CAN_H__
#define CAN_H__

#include "BSP_CAN.h"
#define CARCAN CAN_1 //convenience aliases for the CANBuses
#define MOTORCAN CAN_3
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
	STATE_OF_CHARGE = 0x106
} CANId_t;


typedef union {
	uint8_t b;
	uint16_t h;
	uint32_t w;
	uint64_t d;
} CANData_t;

typedef struct {
	uint8_t idx : 8;
	uint8_t bytes : 8;
	CANData_t data; //TODO: This could probably be replaced with a uint64_t
} CANPayload_t;

/**
 * Data type for message queue
*/
typedef struct {
	CANPayload_t payload;
	CANId_t id;
}CANMSG_t;

typedef enum {CAN_BLOCKING=0, CAN_NON_BLOCKING} CAN_blocking_t;

/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN
 * @return  None
 */
void CANbus_Init(CAN_t bus);

/**
 * @brief   Transmits data onto the CANbus.
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param   blocking: Whether or not the Send should be blocking or not
 * @param   bus: Which bus to transmit on
 * @return  ERROR if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload,CAN_blocking_t blocking, CAN_t bus);


/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers
 * @param   ID pointer to where to store the CAN id of the recieved msg
 * @param   pointer to buffer array to store message. MUST BE 8 BYTES OR LARGER
 * @param   blocking whether or not this read should be blocking
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CANbus_Read(uint32_t *id, uint8_t* buffer, CAN_blocking_t blocking, CAN_t bus);


#endif
