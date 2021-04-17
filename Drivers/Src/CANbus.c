#include "CANbus.h"
#include "config.h"


/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(void) {
    BSP_CAN_Init(CAN_1);
}

/**
 * @brief   Transmits data onto the CANbus
 * @param   CAN bus line
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
int CANbus_Send(CANId_t id, CANPayload_t payload) {

    int8_t data[payload.bytes];

    uint64_t tempData = payload.data.d;
    uint8_t mask = 0xFF;

    for(int i=payload.bytes-1; i>=0; i--){
        data[i] = tempData&mask;
        tempData = tempData>>8;
    }


	return BSP_CAN_Write(CAN_1, id, data, payload.bytes);
}


/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus
 * @param   pointer to buffer array to store message
 * @return  1 if ID matches and 0 if it doesn't
 */

ErrorStatus CANbus_Read(uint8_t* buffer){
    
    uint32_t ID;
    uint8_t data[8];
    uint8_t count = BSP_CAN_Read(CAN_1,&ID,data);
    
    if(ID == CHARGE_ENABLE){
        for(int i=0;i<count;i++){
            buffer[i]=data[i];
        }
        return SUCCESS;
    
    }

    return ERROR;
    
}
