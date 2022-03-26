#include "CANbus.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"

static OS_SEM CANMail_Sem4;       // sem4 to count how many sending hardware mailboxes we have left (start at 3)
static OS_SEM CANBus_RecieveSem4; // sem4 to count how many msgs in our recieving queue
static OS_MUTEX CANbus_TxMutex;   // mutex to lock tx line
static OS_MUTEX CANbus_RxMutex;   // mutex to lock Rx line

/**
 * @brief this function will be passed down to the BSP layer to trigger on RX events. Increments the recieve semaphore to signal message in hardware mailbox. Do not access directly.
 */
void CANbus_RxHandler()
{
    OS_ERR err;
    OSSemPost(&CANBus_RecieveSem4, OS_OPT_POST_1, &err); // increment our queue counter
    assertOSError(0,err);
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox available). Do not access directly.
 */
void CANbus_TxHandler()
{
    OS_ERR err;
    OSSemPost(&CANMail_Sem4, OS_OPT_POST_1, &err);
    assertOSError(0,err);
}

/**
 * @brief   Initializes the CAN system
 * @param   None
 * @return  None
 */
void CANbus_Init(void)
{
    // initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we have
    // initialize tx
    OS_ERR err;

    OSMutexCreate(&CANbus_TxMutex, "CAN TX Lock", &err);
    assertOSError(0,err);

    OSMutexCreate(&CANbus_RxMutex, "CAN RX Lock", &err);
    assertOSError(0,err);

    OSSemCreate(&CANMail_Sem4, "CAN Mailbox Semaphore", 3, &err); // there's 3 hardware mailboxes on the board, so 3 software mailboxes
    assertOSError(0,err);

    OSSemCreate(&CANBus_RecieveSem4, "CAN Recieved Msg queue", 0, &err); // create a mailbox counter to hold the messages in as they come in
    assertOSError(0,err);

    BSP_CAN_Init(CAN_1, &CANbus_RxHandler, &CANbus_TxHandler);
}

/**
 * @brief   Transmits data onto the CANbus
 * @param   id : CAN id of the message
 * @param 	payload : the data that will be sent.
 * @param blocking: Whether or not this should be a blocking call
 * @return  0 if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CANbus_Send(CANId_t id, CANPayload_t payload, CAN_blocking_t blocking)
{
    CPU_TS timestamp;
    OS_ERR err;
    // make sure that Can mailbox is available
    if (blocking == CAN_BLOCKING)
    {
        OSSemPend(
            &CANMail_Sem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
    }
    else
    {
        OSSemPend(
            &CANMail_Sem4,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err);
    }
    assertOSError(0,err);
    if (err != OS_ERR_NONE)
    {
        return ERROR;
    }

    uint8_t txdata[8];
    uint8_t datalen = 0;

    switch (id)
    {
        // Handle 64bit precision case (no idx)
    case MC_BUS:
    case VELOCITY:
    case MC_PHASE_CURRENT:
    case VOLTAGE_VEC:
    case CURRENT_VEC:
    case BACKEMF:
    case TEMPERATURE:
    case ODOMETER_AMPHOURS:
        datalen = 8;
        //TODO: I don't think we need this reversal thing, this was done looking at a logic analyzer trace showing the data frame was reversed
        //We should test reception of data using BPS supplemental ASAP, and if that shows that it's wrong we need to revert this block to the memcpy 
        //that got removed
        txdata[0] = (payload.data.d >> 56) & 0xFF;
        txdata[1] = (payload.data.d >> 48) & 0xFF;
        txdata[2] = (payload.data.d >> 40) & 0xFF;
        txdata[3] = (payload.data.d >> 32) & 0xFF;
        txdata[4] = (payload.data.d >> 24) & 0xFF;
        txdata[5] = (payload.data.d >> 16) & 0xFF;
        txdata[6] = (payload.data.d >> 8) & 0xFF;
        txdata[7] = (payload.data.d >> 0) & 0xFF;
        break;

        // Handle 8bit precision case (0b0000xxxx) (no idx)
    case CAR_STATE:
        datalen = 1;
        txdata[0] = (payload.data.b);
        break;
    case CHARGE_ENABLE:
        //This should never occur, we should never be sending the Charge_enable message out
        return ERROR;
    }

    OSMutexPend( // ensure that tx line is available
        &CANbus_TxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(0,err);
    if (err != OS_ERR_NONE)
    {
        // couldn't lock tx line
        return ERROR;
    }
    // tx line locked

    ErrorStatus retval = BSP_CAN_Write(CAN_1, id, txdata, datalen);

    OSMutexPost( // unlock the TX line
        &CANbus_TxMutex,
        OS_OPT_POST_NONE,
        &err);
    assertOSError(0,err);
    return retval;
}

/**
 * @brief   Checks if the CAN ID matches with expected ID and then copies message to given buffer array
 * @param   CAN line bus (variable buses not implemented, this function currently works with CAN1)
 * @param   ID pointer to where to store the CAN id of the recieved msg
 * @param   pointer to buffer array to store message
 * @param   blocking whether or not this read should be blocking
 * @return  1 if ID matches and 0 if it doesn't
 */

ErrorStatus CANbus_Read(uint32_t *id, uint8_t *buffer, CAN_blocking_t blocking)
{
    CPU_TS timestamp;
    OS_ERR err;

    if (blocking == CAN_BLOCKING)
    {
        OSSemPend( // check if the queue actually has anything
            &CANBus_RecieveSem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &timestamp,
            &err);
    }
    else
    {
        OSSemPend(
            &CANBus_RecieveSem4,
            0,
            OS_OPT_PEND_NON_BLOCKING,
            &timestamp,
            &err);
    }

    assertOSError(0,err);
    if (err != OS_ERR_NONE)
    {
        return ERROR;
    }

    OSMutexPend( // ensure that RX line is available
        &CANbus_RxMutex,
        0,
        OS_OPT_PEND_BLOCKING,
        &timestamp,
        &err);
    assertOSError(0,err);
    if (err != OS_ERR_NONE)
    {
        // couldn't lock RX line
        return ERROR;
    }

    // Actually get the message
    ErrorStatus status = BSP_CAN_Read(CAN_1, id, buffer);

    OSMutexPost( // unlock RX line
        &CANbus_RxMutex,
        OS_OPT_POST_1,
        &err);
    assertOSError(0,err);
    return status;
}
