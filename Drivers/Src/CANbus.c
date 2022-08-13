#include "CANbus.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"

static OS_SEM CANMail_Sem4[NUM_CAN];       // sem4 to count how many sending hardware mailboxes we have left (start at 3)
static OS_SEM CANBus_RecieveSem4[NUM_CAN]; // sem4 to count how many msgs in our recieving queue
static OS_MUTEX CANbus_TxMutex[NUM_CAN];   // mutex to lock tx line
static OS_MUTEX CANbus_RxMutex[NUM_CAN];   // mutex to lock Rx line

/**
 * @brief this function will be passed down to the BSP layer to trigger on RX events. Increments the recieve semaphore to signal message in hardware mailbox. Do not access directly.
 * @param bus The CAN bus to operate on
 */
void CANbus_RxHandler(CAN_t bus)
{
    OS_ERR err;
    OSSemPost(&(CANBus_RecieveSem4[bus]), OS_OPT_POST_1, &err); // increment our queue counter
    assertOSError(OS_CANDRIVER_LOC,err);
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox available). Do not access directly.
 * @param bus The CAN bus to operate on
 */
void CANbus_TxHandler(CAN_t bus)
{
    OS_ERR err;
    OSSemPost(&(CANMail_Sem4[bus]), OS_OPT_POST_1, &err);
    assertOSError(OS_CANDRIVER_LOC,err);
}

//wrapper functions for the interrupt customized for each bus
void CANbus_TxHandler_1(){
    CANbus_TxHandler(CAN_1);
}

void CANbus_TxHandler_3(){
    CANbus_TxHandler(CAN_3);
}

void CANbus_RxHandler_1(){
    CANbus_RxHandler(CAN_1);
}
void CANbus_RxHandler_3(){
    CANbus_RxHandler(CAN_3);
}

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(CAN_t bus)
{
    // initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we have
    // initialize tx
    OS_ERR err;
    
    
    
    OSMutexCreate(&(CANbus_TxMutex[bus]), (bus == CAN_1 ? "CAN TX Lock 1":"CAN TX Lock 3"), &err);
    assertOSError(OS_CANDRIVER_LOC,err);

    OSMutexCreate(&(CANbus_RxMutex[bus]), (bus == CAN_1 ? "CAN RX Lock 1":"CAN RX Lock 3"), &err);
    assertOSError(OS_CANDRIVER_LOC,err);

    OSSemCreate(&(CANMail_Sem4[bus]), (bus == CAN_1 ? "CAN Mailbox Semaphore 1":"CAN Mailbox Semaphore 3"), 3, &err); // there's 3 hardware mailboxes on the board, so 3 software mailboxes
    assertOSError(OS_CANDRIVER_LOC,err);

    OSSemCreate(&(CANBus_RecieveSem4[bus]), (bus == CAN_1 ? "CAN Recieved Msg Queue Ctr 1":"CAN Recieved Msg Queue Ctr 3"), 0, &err); // create a mailbox counter to hold the messages in as they come in
    assertOSError(OS_CANDRIVER_LOC,err);

    if(bus==CAN_1){
        BSP_CAN_Init(bus,&CANbus_RxHandler_1,&CANbus_TxHandler_1);
    } else if (bus==CAN_3){
        BSP_CAN_Init(bus,&CANbus_RxHandler_3,&CANbus_TxHandler_3);
    }

}

/**
 * @brief   Transmits data onto the CANbus. Transmits up to 8 bytes at a time. If more is necessary, please use IDX 
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param blocking: Whether or not this should be a blocking call
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANDATA_t CanData,CAN_blocking_t blocking, CAN_t bus)
{
    CPU_TS timestamp;
    OS_ERR err;

    CANLUT_T msginfo = CANLUT[CanData.ID]; //lookup msg information in table

    //message error checks
    if(msginfo.idxEn & (msginfo.size>7)){ //idx message can only handle up to 7 bytes
        return ERROR;
    } else if (msginfo.size>8){ //non-idx message can only handle up to 8 bytes
        return ERROR;
    } else if (msginfo.size <= 0){ //no 0 size messages allowed
        return ERROR;
    }


    // make sure that Can mailbox is available
    if (blocking == CAN_BLOCKING)
    {
        OSSemPend(
            &(CANMail_Sem4[bus]),
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
        assertOSError(OS_CANDRIVER_LOC,err);
    }
    else
    {
        OSSemPend(
            &(CANMail_Sem4[bus]),
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err);

        // don't crash if we are just using this in non-blocking mode and don't block
        if(err == OS_ERR_PEND_WOULD_BLOCK){
            return ERROR;
        }
        assertOSError(OS_CANDRIVER_LOC,err);
    }


    if (err != OS_ERR_NONE)
    {
        return ERROR;
    }

    uint8_t txdata[8];
    if(msginfo.idxEn){ //first byte of txData should be the idx value
        memcpy(txdata,&CanData.idx,sizeof(CanData.idx));
        memcpy(&(txdata[sizeof(CanData.idx)]),&CanData.data,msginfo.size);
    } else { //non-idx case
        memcpy(txdata,&CanData.data,msginfo.size);
    }

    OSMutexPend( // ensure that tx line is available
        &(CANbus_TxMutex[bus]),
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(OS_CANDRIVER_LOC,err);    // couldn't lock tx line
    
    // tx line locked
    ErrorStatus retval = BSP_CAN_Write(
        bus, //bus to transmit onto 
        CanData.ID, //ID from Data struct
        txdata, //data we memcpy'd earlier
        (msginfo.idxEn ? msginfo.size+sizeof(CanData.idx) : msginfo.size) //if IDX then add one to the msg size, else the msg size
    );

    OSMutexPost( // unlock the TX line
        &(CANbus_TxMutex[bus]),
        OS_OPT_POST_NONE,
        &err);
    assertOSError(OS_CANDRIVER_LOC,err);
    if(retval == ERROR){
        CANbus_TxHandler(bus); //release the mailbox by posting back to the counter semaphore
    }

    return retval;
}

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers. **DOES NOT POPULATE IDXen or IDX. You have to manually inspect the first byte and the ID**
 * @param   ID pointer to where to store the CAN id of the recieved msg
 * @param   pointer to buffer array to store message. MUST BE 8 BYTES OR LARGER
 * @param   blocking whether or not this read should be blocking
 * @returns ERROR if read failed, SUCCESS otherwise
 */

ErrorStatus CANbus_Read(CANDATA_t* MsgContainer, CAN_blocking_t blocking, CAN_t bus)
{
    CPU_TS timestamp;
    OS_ERR err;

    if (blocking == CAN_BLOCKING)
    {
        OSSemPend( // check if the queue actually has anything
            &(CANBus_RecieveSem4[bus]),
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
            assertOSError(OS_CANDRIVER_LOC,err);
    }
    else
    {
        OSSemPend(
            &(CANBus_RecieveSem4[bus]),
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err);

        // don't crash if we are just using this in non-blocking mode and don't block
        if(err == OS_ERR_PEND_WOULD_BLOCK){
            return ERROR;
        }
        assertOSError(OS_CANDRIVER_LOC,err);
    }
    if (err != OS_ERR_NONE)
    {
        return ERROR;
    }

    OSMutexPend( // ensure that RX line is available
        &(CANbus_RxMutex[bus]),
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(OS_CANDRIVER_LOC,err);

    // Actually get the message
    ErrorStatus status = BSP_CAN_Read(bus, (uint32_t*)(&(MsgContainer->ID)), (uint8_t*)(&(MsgContainer->data)));

    OSMutexPost( // unlock RX line
        &(CANbus_RxMutex[bus]),
        OS_OPT_POST_1,
        &err);
    assertOSError(OS_CANDRIVER_LOC,err);
    return status;
}
