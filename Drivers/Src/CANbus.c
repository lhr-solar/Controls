#include "CANbus.h"
#include "config.h"
#include "os.h"
static OS_SEM CANMail_Sem4;
static OS_MUTEX CANbus_TxMutex;



void CANbus_Release(){ 
    //function that releases the mailbox semaphores
    //this will get passed to the BSP_CAN_Init function which will bind the release function to the tx ending handler provided by STFM
};

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(void) {
    //initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we have
    //initialize tx
    OS_ERR err;

	OSMutexCreate(&CANbus_TxMutex, //create mutex to lock the TX line
				  "CAN TX Lock",
				  &err);

	OSSemCreate(&CANMail_Sem4, //create mailbox sem4s to check on send funcs
                "CAN Mailbox Semaphore",
                3,	// Number of mailboxes on the board
                &err);

    BSP_CAN_Init(CAN_1);
}

/**
 * @brief   Transmits data onto the CANbus
 * @param   CAN bus line
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param blocking: Whether or not this should be a blocking call
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload, bool blocking) {
    CPU_TS timestamp;
    OS_ERR err;
    //make sure that Can mailbox is available
    if (blocking){ 
        OSSemPend(
            &CANMail_Sem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err
        );
    } else {
        OSSemPend(
            &CANMail_Sem4,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err
        );
    }
    if (err != OS_ERR_NONE){
        return ERROR;
    }

    
    int8_t data[payload.bytes];
    uint64_t tempData = payload.data.d;
    uint8_t mask = 0xFF;

    for(int i=payload.bytes-1; i>=0; i--){
        data[i] = tempData&mask;
        tempData = tempData>>8;
    }



    OSMutexPend( //ensure that tx write is locked to single write call
        &CANbus_TxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err
    );
    int retval = BSP_CAN_Write(CAN_1, id, data, payload.bytes);

    OSMutexPost( //unlock the TX line
        &CANbus_TxMutex,
        OS_OPT_POST_1,
        &err
    );

    if (retval){
        return SUCCESS;
    } else {
        return ERROR;
    }
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
    
    if(ID == MOTOR_DISABLE){
        for(int i=0;i<count;i++){
            buffer[i]=data[i];
        }
        return SUCCESS;
    
    }

    return ERROR;
    
}


