#include "FaultState.h"


void Task_FaultState(void *p_arg) {
    (void) p_arg;

    OS_ERR err;
    CPU_TS ts;

    // Block until fault is signaled by an assert
    OSSemPend(&FaultState_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
}