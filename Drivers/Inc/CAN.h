#ifndef CAN_H__
#define CAN_H__

typedef enum {
	TRIP = 0x02,
    ALL_CLEAR = 0x101,
    CONTACTOR_STATE = 0x102,
    CURRENT_DATA = 0x103,
    VOLT_DATA = 0x104,
    TEMP_DATA = 0x105,
    SOC_DATA = 0x106,
    WDOG_TRIGGERED = 0x107,
    CAN_ERROR = 0x108
} CANId_t;


typedef union {
	uint8_t b;
	uint16_t h;
	uint32_t w;
	float f;
} CANData_t;

typedef struct {
	uint8_t idx : 8;
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
 * @param   CAN message ID
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */
int CANbus_Read(CAN_t bus, uint32_t id, uint8_t* buffer);

#endif
