/* Copyright (c) 2020 UT Longhorn Racing Solar */
/** @addtogroup CAN_Queue 
 * @{ */

#include "CAN_Queue.h"
#include "os.h"
#include "CANbus.h"
#include "Tasks.h"

// fifo
#define FIFO_TYPE CANDATA_t
#define FIFO_SIZE 256
#define FIFO_NAME CAN_fifo
#include "fifo.h"

static CAN_fifo_t canFifo;
static OS_SEM canFifo_Sem4;
static OS_MUTEX canFifo_Mutex;

void CAN_Queue_Init(void) {
    OS_ERR err;
    CPU_TS ticks;
    OSMutexCreate(&canFifo_Mutex, "CAN queue mutex", &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    OSSemCreate(&canFifo_Sem4,
                "CAN queue semaphore",
                0,
                &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    OSMutexPend(&canFifo_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    CAN_fifo_renew(&canFifo);
    OSMutexPost(&canFifo_Mutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
}

ErrorStatus CAN_Queue_Post(CANDATA_t message) {
    OS_ERR err;
    CPU_TS ticks;
    OSMutexPend(&canFifo_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    bool success = CAN_fifo_put(&canFifo, message);
    OSMutexPost(&canFifo_Mutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    if (success) {
        OSSemPost(&canFifo_Sem4, OS_OPT_POST_1, &err);
        assertOSError(OS_SEND_CAN_LOC, err);
    }

    return success ? SUCCESS : ERROR;
}

ErrorStatus CAN_Queue_Pend(CANDATA_t *message) {
    OS_ERR err;
	CPU_TS ticks;
    
    OSSemPend(&canFifo_Sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    OSMutexPend(&canFifo_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    bool result = CAN_fifo_get(&canFifo, message);
    OSMutexPost(&canFifo_Mutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_SEND_CAN_LOC, err);
    return result ? SUCCESS : ERROR;
}

/*@}*/
