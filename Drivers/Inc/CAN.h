#ifndef CAN_H__
#define CAN_H__

typedef enum {
	MC_BUS = 0x242,
	VELOCITY = 0x243,
	MC_PHASE_CURRENT = 0x244,
	VOLTAGE_VEC = 0x245,
	CURRENT_VEC = 0x246,
	BACKEMF = 0x247,
	TEMPERATURE = 0x24B,
	ODOMETER_AMPHOURS = 0x24E,
	CAR_STATE = 0x580
} CANId_t;

typedef enum{
	MOTOR_DISABLE = 0x10A;
}BPSID_t;

typedef union {
	uint8_t b;
	uint16_t h;
	uint32_t w;
	float f;
	uint64_t d;
} CANData_t;

typedef struct {
	uint8_t idx : 8;
	uint8_t bytes : 8; // Default
	CANData_t data;
} CANPayload_t;

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CAN_Init(void);

/**
 * @brief   Transmits data onto the CANbus
 * @param   CAN bus line
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
int CANbus_Send(CAN_t bus,CANId_t id, CANPayload_t payload);

/**
 * @brief   Checks if the CAN ID matches with Motor disable ID
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */
int CANbus_Read(CAN_t bus, uint8_t* buffer);



#endif
