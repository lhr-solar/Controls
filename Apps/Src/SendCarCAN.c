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


    // uint32_t ids[10] = {MC_BUS, VELOCITY, MC_PHASE_CURRENT, VOLTAGE_VEC, CURRENT_VEC, BACKEMF, TEMPERATURE,
    //                     CAR_STATE, MOTOR_DISABLE, CHARGE_ENABLE};

    // CANData_t data;
    // data.d = 0xFFFF8765;
    

    // CANPayload_t payload;
    // payload.data = data;
    // payload.bytes = 4;

    // int size;

    // size = 0;
    // char str[128];
    // uint8_t output;

    // while (1){
    //     output = CANbus_Send(ids[0], payload, CAN_BLOCKING);
    // }

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