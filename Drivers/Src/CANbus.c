/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANbus.c
 * @brief 
 * 
 */



#include "CANbus.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "CANConfig.h"

static OS_SEM CANMail_Sem4[NUM_CAN];       // sem4 to count how many sending hardware mailboxes we have left (start at 3)
static OS_SEM CANBus_ReceiveSem4[NUM_CAN]; // sem4 to count how many msgs in our recieving queue
static OS_MUTEX CANbus_TxMutex[NUM_CAN];   // mutex to lock tx line
static OS_MUTEX CANbus_RxMutex[NUM_CAN];   // mutex to lock Rx line

/**
 * @brief this function will be passed down to the BSP layer to trigger on RX events. Increments the receive semaphore to signal message in hardware mailbox. Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
 */
void CANbus_RxHandler(CAN_t bus)
{
    OS_ERR err;
    OSSemPost(&(CANBus_ReceiveSem4[bus]), OS_OPT_POST_1, &err); // increment our queue counter
    assertOSError(err);
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox available). Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
 */
void CANbus_TxHandler(CAN_t bus)
{
    OS_ERR err;
    OSSemPost(&(CANMail_Sem4[bus]), OS_OPT_POST_1, &err);
    assertOSError(err);
}

//wrapper functions for the interrupt customized for each bus
void CANbus_TxHandler_2(){
    CANbus_TxHandler(motor);
}

void CANbus_RxHandler_2(){
    CANbus_RxHandler(motor);
}
void CANbus_TxHandler_3(){
    CANbus_TxHandler(car);
}
void CANbus_RxHandler_3(){
    CANbus_RxHandler(car);
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
        if(curr >= MAX_CAN_ID) {
            wlist[i] = 0;
        } else if (CANLUT[curr].size == 0 ) {
            wlist[i] = 0;
        }
    }
    return wlist;
}

ErrorStatus CANbus_Init(CAN_t bus, CANId_t* idWhitelist, uint8_t idWhitelistSize)
{
    // initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we have
    // initialize tx
    OS_ERR err;
    
    OSMutexCreate(&(CANbus_TxMutex[bus]), (bus == motor ? "CAN TX Lock 2":"CAN TX Lock 3"), &err);
    assertOSError(err);

    OSMutexCreate(&(CANbus_RxMutex[bus]), (bus == motor ? "CAN RX Lock 2":"CAN RX Lock 3"), &err);
    assertOSError(err);

    OSSemCreate(&(CANMail_Sem4[bus]), (bus == motor ? "CAN Mailbox Semaphore 2":"CAN Mailbox Semaphore 3"), 3, &err); // there's 3 hardware mailboxes on the board, so 3 software mailboxes
    assertOSError(err);

    OSSemCreate(&(CANBus_ReceiveSem4[bus]), (bus == motor ? "CAN Received Msg Queue Ctr 2":"CAN Received Msg Queue Ctr 3"), 0, &err); // create a mailbox counter to hold the messages in as they come in
    assertOSError(err);

    idWhitelist = whitelist_validator(idWhitelist, idWhitelistSize);
    if(bus==motor){
        BSP_CAN_Init(bus,&CANbus_RxHandler_2,&CANbus_TxHandler_2, (uint16_t*)idWhitelist, idWhitelistSize);
    } else if (bus==car){
        BSP_CAN_Init(bus,&CANbus_RxHandler_3,&CANbus_TxHandler_3, (uint16_t*)idWhitelist, idWhitelistSize);
    } else {
        return ERROR;
    }

    return SUCCESS;
}

ErrorStatus CANbus_Send(CANDATA_t CanData,bool blocking, CAN_t bus)
{
    CPU_TS timestamp;
    OS_ERR err;

    //error check the id
    if(CanData.ID >= MAX_CAN_ID){return ERROR;}

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
        assertOSError(err);
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
    assertOSError(err);    // couldn't lock tx line
    
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
    assertOSError(err);
    if(retval == ERROR){
        CANbus_TxHandler(bus); //release the mailbox by posting back to the counter semaphore
    }

    return retval;
}

ErrorStatus CANbus_Read(CANDATA_t* MsgContainer, bool blocking, CAN_t bus)
{
    CPU_TS timestamp;
    OS_ERR err;

    if (blocking == CAN_BLOCKING)
    {
        OSSemPend( // check if the queue actually has anything
            &(CANBus_ReceiveSem4[bus]),
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
    }
    else
    {
        OSSemPend(
            &(CANBus_ReceiveSem4[bus]),
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
        assertOSError(err);
        return ERROR;
    }

    OSMutexPend( // ensure that RX line is available
        &(CANbus_RxMutex[bus]),
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(err);

    // Actually get the message
    uint32_t id;
    ErrorStatus status = BSP_CAN_Read(bus, &id, MsgContainer->data);

    OSMutexPost( // unlock RX line
        &(CANbus_RxMutex[bus]),
        OS_OPT_POST_NONE,
        &err);
    assertOSError(err);
    if(status == ERROR){
        return ERROR;
    }

    //error check the id
    MsgContainer->ID = (CANId_t) id;
    if(MsgContainer->ID >= MAX_CAN_ID){
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
