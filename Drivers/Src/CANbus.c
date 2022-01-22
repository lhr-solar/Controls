#include "CANbus.h"
#include "config.h"
#include "os.h"


static OS_SEM CANMail_Sem4; //sem4 to count how many sending hardware mailboxes we have left (start at 3)
static OS_SEM CANBus_RecieveSem4; //sem4 to count how many msgs in our recieving queue
static OS_MUTEX CANbus_TxMutex; //mutex to lock tx line 
static OS_MUTEX CANbus_RxMutex; //mutex to lock Rx line


/**
 * @brief this function will be passed down to the BSP layer to trigger on RX events. Increments the recieve semaphore to signal message in hardware mailbox. Do not access directly.
*/
void CANbus_RxHandler(){
    OS_ERR err;
    OSSemPost(&CANBus_RecieveSem4, OS_OPT_POST_1, &err); //increment our queue counter
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox available). Do not access directly.
*/
void CANbus_TxHandler(){
    OS_ERR err;
    OSSemPost(&CANMail_Sem4, OS_OPT_POST_1, &err);
}

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

    OSSemCreate(&CANBus_RecieveSem4, "CAN Recieved Msg queue",0,&err); //create a mailbox counter to hold the messages in as they come in
    BSP_CAN_Init(CAN_1,&CANbus_TxHandler,&CANbus_RxHandler);
}


/**
 * @brief   Transmits data onto the CANbus
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param blocking: Whether or not this should be a blocking call
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload, CAN_blocking_t blocking) {
    CPU_TS timestamp;
    OS_ERR err;
    //make sure that Can mailbox is available
    if (blocking == CAN_BLOCKING){ 
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
            memcpy(&txdata,&payload.data.d,sizeof(payload.data.d));


    //Handle 8bit precision case (0b0000xxxx) (no idx)
       case CAR_STATE:
            datalen = 1;
            memcpy(&txdata,&payload.data.b,sizeof(payload.data.b)); //copy into txdata array  the 8 bits of the payload

    }

    OSMutexPend( //ensure that tx line is available
        &CANbus_TxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err
    );
    if (err != OS_ERR_NONE){
        //couldn't lock tx line
        return ERROR;
    }
    //tx line locked

    int retval = BSP_CAN_Write(CAN_1, id, txdata, datalen);

    OSMutexPost( //unlock the TX line
        &CANbus_TxMutex,
        OS_OPT_POST_NONE,
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
 * @param   CAN line bus (variable buses not implemented, this function currently works with CAN1)
 * @param   ID pointer to where to store the CAN id of the recieved msg
 * @param   pointer to buffer array to store message
 * @param   blocking whether or not this read should be blocking
 * @return  1 if ID matches and 0 if it doesn't
 */

ErrorStatus CANbus_Read(uint8_t* buffer,CAN_blocking_t blocking){
    CPU_TS timestamp;
    OS_ERR err;

    if(blocking == CAN_BLOCKING){
        OSSemPend( //check if the queue actually has anything
            &CANBus_RecieveSem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err
        );
    } else {
        OSSemPend(
            &CANBus_RecieveSem4,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err
        );
    }
    if(err != OS_ERR_NONE) {
        return ERROR;
    }

    OSMutexPend( //ensure that RX line is available
        &CANbus_RxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err
    );
    if (err != OS_ERR_NONE){ 
        //couldn't lock RX line
        return ERROR;
    }

    //GETMSG
    CANId_t ret;
    uint8_t status = BSP_CAN_Read(CAN_1,&ret,buffer);
    

    OSMutexPost( //unlock RX line
        &CANbus_RxMutex,
        OS_OPT_POST_1,
        &err
    );
    if(status&&(ret == CHARGE_ENABLE)){
        return SUCCESS;
    } else {
        return ERROR;
    }

}


