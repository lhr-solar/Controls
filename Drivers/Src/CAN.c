#include "CAN.h"

static void floatToBytes(float val, uint8_t* bytes_array, uint8_t bytes);

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */

// Not sure if we want CAN1 to be an argument to CAN_Init or not
void CAN_Init(void) {
    BSP_CAN_Init(CAN1);
}

/**
 * @brief   Transmits data onto the CANbus
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
int CANbus_Send(CANId_t id, CANPayload_t payload) {

	uint8_t bytes = 4; // Not sure how we want to decide the number of wanted bytes. An extra paramter to this function?
	

	// Not sure if these are the ids we want but I took them from BPS as placeholders
	switch (id) {
		case TRIP:
			return BSP_CAN_Write(id, &payload.data.b, 1);

		case ALL_CLEAR:
			return BSP_CAN_Write(id, &payload.data.b, 1);

		case CONTACTOR_STATE:
			return BSP_CAN_Write(id, &payload.data.b, 1);

		case CURRENT_DATA:
			uint8_t txdata[bytes];
			floatToBytes(payload.data.f, &txdata[0]);
			return BSP_CAN_Write(id, txdata, bytes);

		case VOLT_DATA:
		case TEMP_DATA:
			uint8_t txdata[bytes+1];
			txdata[0] = payload.idx;
			floatToBytes(payload.data.f, &txdata[1]);
			return BSP_CAN_Write(id, txdata, bytes);

		case SOC_DATA:
			uint8_t txdata[bytes];
			floatToBytes(payload.data.f, &txdata[0]);
			return BSP_CAN_Write(id, txdata, bytes);

		case WDOG_TRIGGERED:
			return BSP_CAN_Write(id, &payload.data.b, 1);

		case CAN_ERROR:
			return BSP_CAN_Write(id, &payload.data.b, 1);
	}
	return 0;
}

/**
 * @brief   Converts float to array of designated number of bytes
 * @param   val : The flaot you want to convert
 * @param	bytes_array: Pointer to array where you want the bytes to be put
 * @param   bytes : The number of bytes you want the float to be converted into.
 * @return  None
 */

static void floatToBytes(float val, uint8_t* bytes_array, uint8_t bytes) {
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
}

/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus
 * @param   CAN message ID
 * @param   pointer to buffer array to store message
 * @return  0 if ID matches and 1 if it doesn't
 */

int CANbus_Read(CAN_t bus, uint32_t Expected_ID, uint8_t* buffer)
{
    uint32_t ID;
    uint8_t data[];
    uint8_t count = BSP_CAN_READ(bus,*ID,data);
    if(Expected_ID == ID)
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
