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
	CAR_STATE = 0x580,
	MOTOR_DISABLE = 0x10A
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
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(void);

/**
 * @brief   Transmits data onto the CANbus
 * @param   CAN bus line
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
int CANbus_Send(CANId_t id, CANPayload_t payload);

/**
 * @brief   Checks if the CAN ID matches with Motor disable ID
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  1 if ID matches and 0 if it doesn't
 */
ErrorStatus CANbus_Read(uint8_t* buffer);



#endif
