/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANbus.c
 * @brief
 *
 */

#include "CanBus.h"

#include "CanConfig.h"
#include "Tasks.h"
#include "config.h"
#include "os.h"

static OS_SEM
    can_mail_sem4[kNumCan];  // sem4 to count how many sending hardware
                             // mailboxes we have left (start at 3)
static OS_SEM can_bus_receive_sem4[kNumCan];  // sem4 to count how many msgs in
                                              // our recieving queue
static OS_MUTEX can_bus_tx_mutex[kNumCan];    // mutex to lock tx line
static OS_MUTEX can_bus_rx_mutex[kNumCan];    // mutex to lock Rx line

/**
 * @brief this function will be passed down to the BSP layer to trigger on RX
 * events. Increments the receive semaphore to signal message in hardware
 * mailbox. Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
 */
void CaNbusRxHandler(Can bus) {
    OS_ERR err = 0;
    OSSemPost(&(can_bus_receive_sem4[bus]), OS_OPT_POST_1,
              &err);  // increment our queue counter
    ASSERT_OS_ERROR(err);
}

/**
 * @brief this function will be passed down to the BSP layer to trigger on
 * TXend. Releases hold of the mailbox semaphore (Increments it to show mailbox
 * available). Do not access directly outside this driver.
 * @param bus The CAN bus to operate on. Should be CARCAN or MOTORCAN.
 */
void CaNbusTxHandler(Can bus) {
    OS_ERR err = 0;
    OSSemPost(&(can_mail_sem4[bus]), OS_OPT_POST_1, &err);
    ASSERT_OS_ERROR(err);
}

// wrapper functions for the interrupt customized for each bus
void CaNbusTxHandler1() { CaNbusTxHandler(kCan1); }

void CaNbusRxHandler1() { CaNbusRxHandler(kCan1); }
void CaNbusTxHandler3() { CaNbusTxHandler(kCan3); }
void CaNbusRxHandler3() { CaNbusRxHandler(kCan3); }

/**
 * @brief Checks each CAN ID. If an ID is not in kCanLut, set that ID to NULL
 * @param wlist The whitelist containing the IDs to be checked
 * @param size The size of the whitelist of IDs
 * @return Returns the whitelist to be
 */
static CanId* whitelistValidator(CanId* wlist, uint8_t size) {
    for (int i = 0; i < size; i++) {
        CanId curr = wlist[i];
        if (curr >= kMaxCanId || kCanLut[curr].size == 0) {
            wlist[i] = 0;
        }
    }
    return wlist;
}

ErrorStatus CanBusInit(Can bus, CanId* id_whitelist,
                       uint8_t id_whitelist_size) {
    // initialize CAN mailbox semaphore to 3 for the 3 CAN mailboxes that we
    // have initialize tx
    OS_ERR err = 0;
    id_whitelist = whitelistValidator(id_whitelist, id_whitelist_size);
    if (bus == kCan1) {
        BspCanInit(bus, &CaNbusRxHandler1, &CaNbusTxHandler1,
                   (uint16_t*)id_whitelist, id_whitelist_size);
    } else if (bus == kCan3) {
        BspCanInit(bus, &CaNbusRxHandler3, &CaNbusTxHandler3,
                   (uint16_t*)id_whitelist, id_whitelist_size);
    } else {
        return ERROR;
    }

    OSMutexCreate(&(can_bus_tx_mutex[bus]),
                  (bus == kCan1 ? "CAN TX Lock 1" : "CAN TX Lock 3"), &err);
    ASSERT_OS_ERROR(err);

    OSMutexCreate(&(can_bus_rx_mutex[bus]),
                  (bus == kCan1 ? "CAN RX Lock 1" : "CAN RX Lock 3"), &err);
    ASSERT_OS_ERROR(err);

    OSSemCreate(
        &(can_mail_sem4[bus]),
        (bus == kCan1 ? "CAN Mailbox Semaphore 1" : "CAN Mailbox Semaphore 3"),
        BSP_CAN_NUM_MAILBOX, &err);  // there's 3 hardware mailboxes on the
                                     // board, so 3 software mailboxes
    ASSERT_OS_ERROR(err);

    OSSemCreate(&(can_bus_receive_sem4[bus]),
                (bus == kCan1 ? "CAN Received Msg Queue Ctr 1"
                              : "CAN Received Msg Queue Ctr 3"),
                0, &err);  // create a mailbox counter to hold the messages in
                           // as they come in
    ASSERT_OS_ERROR(err);

    return SUCCESS;
}

ErrorStatus CanBusSend(CanData can_data, bool blocking, Can bus) {
    CPU_TS timestamp = 0;
    OS_ERR err = 0;

    // error check the id
    if (can_data.id >= kMaxCanId) {
        return ERROR;
    }

    CanLut msginfo = kCanLut[can_data.id];  // lookup msg information in table

    if (msginfo.size == 0) {
        return ERROR;
    }  // if they passed in an invalid id, it will be zero

    // make sure that Can mailbox is available
    if (blocking == CAN_BLOCKING) {
        OSSemPend(&(can_mail_sem4[bus]), 0, OS_OPT_PEND_BLOCKING, &timestamp,
                  &err);
    } else {
        OSSemPend(&(can_mail_sem4[bus]), 0, OS_OPT_PEND_NON_BLOCKING,
                  &timestamp, &err);

        // don't crash if we are just using this in non-blocking mode and don't
        // block
        if (err == OS_ERR_PEND_WOULD_BLOCK) {
            return ERROR;
        }
    }
    if (err != OS_ERR_NONE) {
        ASSERT_OS_ERROR(err);
        return ERROR;
    }

    uint8_t txdata[BSP_CAN_DATA_LENGTH];
    if (msginfo.idx_en) {  // first byte of txData should be the idx value
        memcpy(txdata, &can_data.idx, 1);
        memcpy(&(txdata[sizeof(can_data.idx)]), &can_data.data, msginfo.size);
    } else {  // non-idx case
        memcpy(txdata, &can_data.data, msginfo.size);
    }

    OSMutexPend(  // ensure that tx line is available
        &(can_bus_tx_mutex[bus]), 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    ASSERT_OS_ERROR(err);  // couldn't lock tx line

    // tx line locked
    ErrorStatus retval = BspCanWrite(
        bus,          // bus to transmit onto
        can_data.id,  // ID from Data struct
        txdata,       // data we memcpy'd earlier
        (msginfo.idx_en ? msginfo.size + sizeof(can_data.idx)
                        : msginfo.size)  // if IDX then add one to the msg size,
                                         // else the msg size
    );

    OSMutexPost(  // unlock the TX line
        &(can_bus_tx_mutex[bus]), OS_OPT_POST_NONE, &err);
    ASSERT_OS_ERROR(err);
    if (retval == ERROR) {
        CaNbusTxHandler(bus);  // release the mailbox by posting back to the
                               // counter semaphore
    }

    return retval;
}

ErrorStatus CanBusRead(CanData* msg_container, bool blocking, Can bus) {
    CPU_TS timestamp = 0;
    OS_ERR err = 0;

    if (blocking == CAN_BLOCKING) {
        OSSemPend(  // check if the queue actually has anything
            &(can_bus_receive_sem4[bus]), 0, OS_OPT_PEND_BLOCKING, &timestamp,
            &err);
    } else {
        OSSemPend(&(can_bus_receive_sem4[bus]), 0, OS_OPT_PEND_NON_BLOCKING,
                  &timestamp, &err);

        // don't crash if we are just using this in non-blocking mode and don't
        // block
        if (err == OS_ERR_PEND_WOULD_BLOCK) {
            return ERROR;
        }
    }
    if (err != OS_ERR_NONE) {
        ASSERT_OS_ERROR(err);
        return ERROR;
    }

    OSMutexPend(  // ensure that RX line is available
        &(can_bus_rx_mutex[bus]), 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    ASSERT_OS_ERROR(err);

    // Actually get the message
    uint32_t id = 0;
    ErrorStatus status = BspCanRead(bus, &id, msg_container->data);

    OSMutexPost(  // unlock RX line
        &(can_bus_rx_mutex[bus]), OS_OPT_POST_NONE, &err);
    ASSERT_OS_ERROR(err);
    if (status == ERROR) {
        return ERROR;
    }

    // error check the id
    msg_container->id = (CanId)id;
    if (msg_container->id >= kMaxCanId) {
        msg_container = NULL;
        return ERROR;
    }
    CanLut entry =
        kCanLut[msg_container->id];  // lookup msg information in table
    if (entry.size == 0) {
        msg_container = NULL;
        return ERROR;
    }  // if they passed in an invalid id, it will be zero

    // search LUT for id to populate idx and trim data
    if (entry.idx_en == true) {
        msg_container->idx = msg_container->data[0];
        memmove(  // Can't use memcpy, as memory regions overlap
            msg_container->data, &(msg_container->data[1]),
            BSP_CAN_DATA_LENGTH -
                1  // max size of data (8) - size of idx byte (1)
        );
    }
    return status;
}
