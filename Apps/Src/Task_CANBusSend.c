#include "os.h"
#include "Tasks.h"
#include "bsp.h"
#include "CANbus.h"

void Task_SendCarCAN(void *p_arg){
    OS_ERR err;
    OS_MSG_SIZE msgSize;
    CPU_TS timestamp;
    CANMSG_t *msg;
    CANbus_Init();

    while (1){
        (msg) = (CANMSG_t*)OSQPend(
            &CANBus_MsgQ,
            0,
            OS_OPT_PEND_BLOCKING,
            &msgSize,
            &timestamp,
            &err
        );
        if (err == OS_ERR_NONE){ //if a message is recieved
            CANbus_Send(msg->id,msg->payload,CAN_BLOCKING); //send message
        }
    }
    
}