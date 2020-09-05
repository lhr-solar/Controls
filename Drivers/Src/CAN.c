#include "CAN.h"

static void floatToBytes(float val, uint8_t* bytes_array, uint8_t bytes);

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CAN_Init(void) {
    BSP_CAN_Init(CAN1);
}

/**
 * @brief   Transmits data onto the CANbus
 * @param   CAN bus line
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
int CAN_Send(CAN_t bus,CANId_t id, CANPayload_t payload) {

	// Not sure if these are the ids we want but I took them from BPS as placeholders
	switch (id) {
		case MC_BUS:
		case VELOCITY:
		case MC_PHASE_CURRENT:
		case VOLTAGE_VEC:
		case CURRENT_VEC:
		case BACKEMF:
		case TEMPERATURE:
		case ODOMETER_AMPHOURS:
			return BSP_CAN_WRITE(bus,id, &payload.data.d, payload.bytes)
		case CAR_STATE:
			return BSP_CAN_WRITE(bus,id, &payload.data.b, payload.bytes)
                default:
			return 0;

	}
}

/**
 * @brief   Converts float to array of designated number of bytes
 * @param   val : The flaot you want to convert
 * @param	bytes_array: Pointer to array where you want the bytes to be put
 * @param   bytes : The number of bytes you want the float to be converted into.
 * @return  None
 */

/*static void floatToBytes(float val, uint8_t* bytes_array, uint8_t bytes) {
	uint8_t temp;
	// Create union of shared memory space
	union {
			float float_variable;
			uint8_t temp_array[bytes];
	} u;
	// Overwrite bytes of union with float variable
	u.float_variable = val;
	// Assign bytes to input array
	memcpy(bytes_array, u.temp_array, bytes);

	// Reverse the array. I don't know why this has to be done, to be honest, but that's what
	// they did in floatTo4Bytes in BPS/Drivers/Src/CANbus.c . This is a generalized version of
	// what was done in that file so that we can reverse an array of any length.
	for(i=0; i<bytes/2; i++){
		temp = bytes_array[i];
		bytes_array[i] = bytes_array[bytes-1-i]
		bytes_array[bytes-1-i] = temp 
	}
}*/

/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */

int CANbus_Read(CAN_t bus, uint8_t* buffer)
{
    // ID 0x10A is Motor Disable
    uint32_t ID;
    uint8_t data[];
    uint8_t count = BSP_CAN_READ(bus,&ID,data);
    if(ID == MOTOR_DISABLE)
    {
        for(int i=0;i<count;i++)
        {
            buffer[i]=data[i];
        }
	return 0;
    }
    else
    {
      return 1;
    }
    
}
