#include "CANbus.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "CANConfig.h"

static OS_SEM CANMail_Sem4[NUM_CAN];       // sem4 to count how many sending hardware mailboxes we have left (start at 3)
static OS_SEM CANBus_RecieveSem4[NUM_CAN]; // sem4 to count how many msgs in our recieving queue
static OS_MUTEX CANbus_TxMutex[NUM_CAN];   // mutex to lock tx line
static OS_MUTEX CANbus_RxMutex[NUM_CAN];   // mutex to lock Rx line

/** @ingroup FooGroup 
 * @{
*/


/**
 * @brief this function will be passed down to the BSP layer to trigger on RX events. Increments the recieve semaphore to signal message in hardware mailbox. Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
 */
void CANbus_RxHandler(CAN_t bus)
{
    OS_ERR err;
    OSSemPost(&(CANBus_RecieveSem4[bus]), OS_OPT_POST_1, &err); // increment our queue counter
    assertOSError(OS_CANDRIVER_LOC,err);
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox available). Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
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

void CANbus_RxHandler_1(){
    CANbus_RxHandler(CAN_1);
}
void CANbus_TxHandler_3(){
    CANbus_TxHandler(CAN_3);
}
void CANbus_RxHandler_3(){
    CANbus_RxHandler(CAN_3);
}

/**
 * @brief Checks each CAN ID. If an ID is not in CANLUT, set that ID to NULL
 * @param wlist The whitelist containing the IDs to be checked
 * @param size The size of the whitelist of IDs
 * @return Returns the whitelist to be 
*/
static CANId_t* whitelist_validator(CANId_t* wlist, uint8_t size){
    for(int i = 0; i < size; i++){
        CANId_t curr = wlist[i];
        if(curr >= NUM_CAN_IDS) {
            wlist[i] = 0;
        } else if (CANLUT[curr].size == 0 ) {
            wlist[i] = 0;
        }
    }
    return wlist;
}
/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN. CAN2 will not be supported.
 * @param   idWhitelist A list of CAN IDs that we want to receive. If NULL, we will receive all messages.
 * @param   idWhitelistSize The size of the whitelist.
 * @return  ERROR if bus != CAN1 or CAN3, SUCCESS otherwise
 */
ErrorStatus CANbus_Init(CAN_t bus, CANId_t* idWhitelist, uint8_t idWhitelistSize)
{
    // initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we have
    // initialize tx
    OS_ERR err;
    idWhitelist = whitelist_validator(idWhitelist, idWhitelistSize);
    if(bus==CAN_1){
        BSP_CAN_Init(bus,&CANbus_RxHandler_1,&CANbus_TxHandler_1, (uint16_t*)idWhitelist, idWhitelistSize);
    } else if (bus==CAN_3){
        BSP_CAN_Init(bus,&CANbus_RxHandler_3,&CANbus_TxHandler_3, (uint16_t*)idWhitelist, idWhitelistSize);
    } else {
        return ERROR;
    }

    OSMutexCreate(&(CANbus_TxMutex[bus]), (bus == CAN_1 ? "CAN TX Lock 1":"CAN TX Lock 3"), &err);
    assertOSError(OS_CANDRIVER_LOC,err);

    OSMutexCreate(&(CANbus_RxMutex[bus]), (bus == CAN_1 ? "CAN RX Lock 1":"CAN RX Lock 3"), &err);
    assertOSError(OS_CANDRIVER_LOC,err);

    OSSemCreate(&(CANMail_Sem4[bus]), (bus == CAN_1 ? "CAN Mailbox Semaphore 1":"CAN Mailbox Semaphore 3"), 3, &err); // there's 3 hardware mailboxes on the board, so 3 software mailboxes
    assertOSError(OS_CANDRIVER_LOC,err);

    OSSemCreate(&(CANBus_RecieveSem4[bus]), (bus == CAN_1 ? "CAN Recieved Msg Queue Ctr 1":"CAN Recieved Msg Queue Ctr 3"), 0, &err); // create a mailbox counter to hold the messages in as they come in
    assertOSError(OS_CANDRIVER_LOC,err);

    return SUCCESS;
}

/**
 * @brief   Transmits data onto the specified CANbus. Transmits up to 8 bytes at a time. If more is necessary, please use an IDX message.
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param   blocking: Whether or not this should be a blocking call
 * @return  ERROR if data wasn't sent, SUCCESS if it was sent.
 */
ErrorStatus CANbus_Send(CANDATA_t CanData,bool blocking, CAN_t bus)
{
    CPU_TS timestamp;
    OS_ERR err;

    //error check the id
    if(CanData.ID >= NUM_CAN_IDS){return ERROR;}

    CANLUT_T msginfo = CANLUT[CanData.ID]; //lookup msg information in table
    
    if(msginfo.size == 0){return ERROR;} //if they passed in an invalid id, it will be zero


    // make sure that Can mailbox is available
    if (blocking == CAN_BLOCKING)
    {
        OSSemPend(
            &(CANMail_Sem4[bus]),
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
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
    }
    if (err != OS_ERR_NONE)
    {
        assertOSError(OS_CANDRIVER_LOC,err);
        return ERROR;
    }

    uint8_t txdata[8];
    if(msginfo.idxEn){ //first byte of txData should be the idx value
        memcpy(txdata, &CanData.idx, 1);
        memcpy(&(txdata[sizeof(CanData.idx)]), &CanData.data, msginfo.size);
    } else { //non-idx case
        memcpy(txdata, &CanData.data, msginfo.size);
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
 * @param   MsgContainer Where to store the recieved message
 * @param   blocking     Whether or not this read should be blocking
 * @param   bus          Which bus to read a message from
 * @returns              ERROR if read failed, SUCCESS otherwise
 */

ErrorStatus CANbus_Read(CANDATA_t* MsgContainer, bool blocking, CAN_t bus)
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
    }
    if (err != OS_ERR_NONE)
    {
        assertOSError(OS_CANDRIVER_LOC,err);
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
    uint32_t id;
    ErrorStatus status = BSP_CAN_Read(bus, &id, MsgContainer->data);

    OSMutexPost( // unlock RX line
        &(CANbus_RxMutex[bus]),
        OS_OPT_POST_NONE,
        &err);
    assertOSError(OS_CANDRIVER_LOC,err);
    if(status == ERROR){
        return ERROR;
    }

    //error check the id
    MsgContainer->ID = (CANId_t) id;
    if(MsgContainer->ID >= NUM_CAN_IDS){
        MsgContainer = NULL;
        return ERROR;
    }
    CANLUT_T entry = CANLUT[MsgContainer->ID]; //lookup msg information in table
    if(entry.size == 0){
        MsgContainer = NULL;
        return ERROR;
    } //if they passed in an invalid id, it will be zero
    
    //search LUT for id to populate idx and trim data
    if(entry.idxEn==true){
        MsgContainer->idx = MsgContainer->data[0];
        memmove( // Can't use memcpy, as memory regions overlap
            MsgContainer->data,
            &(MsgContainer->data[1]),
            7 // max size of data (8) - size of idx byte (1)
        );
    }
    return status;
}
/*@}*/

