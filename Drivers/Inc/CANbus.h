/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef CAN_H__
#define CAN_H__

#include "BSP_CAN.h"

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
	CANData_t data;
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
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(void);

/**
 * @brief   Transmits data onto the CANbus.
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param   blocking: Whether or not the Send should be blocking or not
 * @return  ERROR if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload,CAN_blocking_t blocking);


/**
 * @brief   Checks if the CAN ID matches with Motor disable ID
 * @param   canline (not implemented, default is CAN1) can line to read from
 * @param 	id CAN msg ID
 * @param 	buffer pointer to buffer in which to store the can msg
 * @param   blocking whether or not this Read should be a blocking read or a nonblocking read
 * @return  1 if ID matches and 0 if it doesn't
 */
ErrorStatus CANbus_Read(uint32_t *id, uint8_t* buffer, CAN_blocking_t blocking);


#endif
