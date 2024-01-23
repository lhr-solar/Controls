/**
 * @file SendCarCan.c
 *
 * The Send Car Can task is a simple queue consumer task. Multiple
 * tasks need to write to the car CAN bus; in order to do this safely,
 * they append their messages to a CAN queue. The Send Car Can task simply pends
 * on this queue and forwards messages to the Car CAN bus when any arrive.
 *
 * The tasks that produce messages for the SendCarCAN queue include:
 * - [ReadTritium](./ReadTritium.html) (all messages on MotorCAN bus are echoed
 * across CarCAN bus)
 * - [SendTritium](./SendTritium.html) (the current FSM state is echoed across
 * CarCAN bus for logging)
 * - PutIOState (the current IO state is echoed across CarCAN bus for logging
 * and for the ignition sequence)
 *
 * # Put IO State Task
 * The Put IO State task puts the current IO state on the CAN bus. It is used to
 * send the IO state to Data Acquisition (for logging purposes) and the BPS (for
 * ignition sequence purposes). Currently, it is written within SendCarCan.c. It
 * is a separate task from SendCarCan (subject to change).
 *
 */

#include "SendCarCan.h"

#include "CanBus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"
#include "SendTritium.h"
#include "Tasks.h"
#include "common.h"
#include "os_cfg_app.h"

/**
 * @brief Period of the IO state message in milliseconds
 */
#define IO_STATE_DLY_MS 250u

#define SENDCARCAN_MSG_SKIP_CTR 3

// Task_PutIOState
OS_TCB put_io_state_tcb;
CPU_STK put_io_state_stk[TASK_SEND_CAR_CAN_STACK_SIZE];

// SendCarCAN Queue
#define FIFO_TYPE CanData
#define FIFO_SIZE 50
#define FIFO_NAME Queue
#include "fifo.h"

static Queue can_fifo;

static OS_SEM car_can_sem4;
static OS_MUTEX car_can_mtx;

static void taskPutIoState(void *p_arg);

/**
 * @brief return the space left in Queue for debug purposes
 */
#ifdef DEBUG
uint8_t GetSendCarCanQueueSpace(void) {
    return (can_fifo.get - can_fifo.put - 1) %
           (sizeof can_fifo.buffer / sizeof can_fifo.buffer[0]);
}
#endif

/**
 * @brief Wrapper to put new message in the CAN queue
 */
void SendCarCanPut(CanData message) {
    OS_ERR err = OS_ERR_NONE;
    CPU_TS ticks = 0;
    bool success = false;

    static uint8_t carcan_ctr = 0;

    if (carcan_ctr > SENDCARCAN_MSG_SKIP_CTR) {
        OSMutexPend(&car_can_mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        ASSERT_OS_ERROR(err);

        success = QueuePut(&can_fifo, message);

        OSMutexPost(&car_can_mtx, OS_OPT_POST_NONE, &err);
        ASSERT_OS_ERROR(err);

        carcan_ctr = 0;
    }
    carcan_ctr++;

    if (success) {
        OSSemPost(&car_can_sem4, OS_OPT_POST_1, &err);
        ASSERT_OS_ERROR(err);
    }
}

/**
 * @brief Initialize SendCarCAN
 */
void SendCarCanInit(void) {
    OS_ERR err = 0;

    OSMutexCreate(&car_can_mtx, "CarCAN_Mtx", &err);
    ASSERT_OS_ERROR(err);

    OSSemCreate(&car_can_sem4, "CarCAN_Sem4", 0, &err);
    ASSERT_OS_ERROR(err);

    QueueRenew(&can_fifo);
}

/**
 * @brief Grabs the latest messages from the queue and sends over CarCAN
 */
void TaskSendCarCan(void *p_arg) {
    OS_ERR err = 0;
    CPU_TS ticks = 0;

    CanData message;
    memset(&message, 0, sizeof message);

    // PutIOState
    OSTaskCreate((OS_TCB *)&put_io_state_tcb, (CPU_CHAR *)"PutIOState",
                 (OS_TASK_PTR)taskPutIoState, (void *)NULL,
                 (OS_PRIO)TASK_PUT_IOSTATE_PRIO, (CPU_STK *)put_io_state_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                 (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE, (OS_MSG_QTY)0,
                 (OS_TICK)0, (void *)NULL, (OS_OPT)(OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    while (1) {
        // Check if there's something to send in the queue (either IOState or
        // Car state from sendTritium)
        OSSemPend(&car_can_sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        ASSERT_OS_ERROR(err);

        OSMutexPend(&car_can_mtx, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
        ASSERT_OS_ERROR(err);

        bool res = QueueGet(&can_fifo, &message);

        OSMutexPost(&car_can_mtx, OS_OPT_POST_NONE, &err);
        ASSERT_OS_ERROR(err);

        if (res) {
            CanBusSend(message, true, CARCAN);
        }
    }
}

/**
 * @brief Sends IO information over CarCAN
 */
static void putIOState(void) {
    CanData message;
    memset(&message, 0, sizeof message);
    message.id = kIoState;

    // Get pedal information
    message.data[0] = PedalsRead(kAccelerator);
    message.data[1] = PedalsRead(kBrake);

    // Get minion information
    for (Pin pin = 0; pin < kNumPins; pin++) {
        bool pin_state = MinionsRead(pin);
        message.data[2] |= pin_state << pin;
    }

    // Get contactor info
    for (Contactor contactor = 0; contactor < kNumContactors; contactor++) {
        bool contactor_state = (ContactorsGet(contactor) == ON) ? true : false;
        message.data[3] |= contactor_state << contactor;
    }

    // Tell BPS if the array contactor should be on
    message.data[3] |= (!MinionsRead(kIgn1) || !MinionsRead(kIgn2)) << 2;

    CanBusSend(message, true, CARCAN);
}

/**
 * @brief Sends IO information over CarCAN every IO_STATE_DLY_MS
 */
static void taskPutIoState(void *p_arg) {
    OS_ERR err = 0;

    while (1) {
        putIOState();
        OSTimeDlyHMSM(0, 0, 0, IO_STATE_DLY_MS, OS_OPT_TIME_HMSM_STRICT, &err);
        ASSERT_OS_ERROR(err);
    }
}
