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


typedef union {
	uint8_t b;
	uint16_t h;
	uint32_t w;
	float f;
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
 * @param   id : CAN id of the message
 * @param   payload : the data that will be sent.
 */
int CAN_Send(CANId_t id, CANPayload_t payload);

/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */
<<<<<<< Updated upstream
int CANbus_Read(CAN_t bus, uint8_t* buffer);
=======
int CAN_Read(CAN_t bus, uint32_t id, uint8_t* buffer);
>>>>>>> Stashed changes

#endif
