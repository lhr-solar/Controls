#include "CANbus.h"
#include "config.h"
#include "os.h"
static OS_SEM CANMail_Sem4;
static OS_SEM CANBus_RecieveQ; 
static OS_MUTEX CANbus_TxMutex;
static OS_MUTEX CANbus_RxMutex;

//function that releases the mailbox semaphores
//this will get passed to the BSP_CAN_Init function which will bind the release function to the tx ending handler provided by STFM
void CANbus_Release(){ 
    
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

	OSMutexCreate(&CANbus_TxMutex,"CAN TX Lock",&err);

    OSMutexCreate(&CANbus_RxMutex,"CAN RX Lock",&err);

	OSSemCreate(&CANMail_Sem4, "CAN Mailbox Semaphore", 3,&err); //there's 3 hardware mailboxes on the board, so 3 software mailboxes

    OSSemCreate(&CANBus_RecieveQ, "CAN Recieved Msg queue",0,&err); //create a queue to hold the messages in as they come in
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
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload, CAN_blocking_t blocking) {
    CPU_TS timestamp;
    OS_ERR err;
    //make sure that Can mailbox is available
    if (blocking == CANBLOCKING){ 
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

    //OLD CANWRITING SYS
    int8_t data[payload.bytes];
    uint64_t tempData = payload.data.d;
    uint8_t mask = 0xFF;

    for(int i=payload.bytes-1; i>=0; i--){
        data[i] = tempData&mask;
        tempData = tempData>>8;
    }

    //NEW CANWRITING SYS
    uint8_t txdata[8];
    uint8_t datalen;

    switch(id){
    //Handle 64bit precision case (no idx)
       case MC_BUS:
       case VELOCITY:
       case MC_PHASE_CURRENT:
       case VOLTAGE_VEC:
       case CURRENT_VEC:
       case BACKEMF:
       case TEMPERATURE:
       case ODOMETER_AMPHOURS:
            datalen = 8;
            memcpy(&txdata,payload.data.d,sizeof(payload.data.d));

    //Handle 8bit precision case (0b0000xxxx) (no idx)
       case CAR_STATE:
            uint8_t datalen = 1;
            memcpy(&txdata,payload.data.b,sizeof(payload.data.b)); //copy into txdata array  the 8 bits of the payload

    }



    OSMutexPend( //ensure that tx write is locked to single write call
        &CANbus_TxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err
    );

    int retval = BSP_CAN_Write(CAN_1, id, txdata, datalen);

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


