#ifndef CAN_H__
#define CAN_H__

typedef enum {
    MOTOR_DRIVE = 0x221,
    MOTOR_POWER = 0x222,
    RESET = 0x223,
    VELOCITY = 0x243
    MOTOR_DISABLE = 0x10A;
	  
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
void CANbus_Init(void);

/**
 * @brief   Transmits data onto the CANbus
 * @param   id : CAN id of the message
 * @param   payload : the data that will be sent.
 */
int CANbus_Send(CANId_t id, CANPayload_t payload);

/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */
int CANbus_Read(CAN_t bus, uint8_t* buffer);

#endif
