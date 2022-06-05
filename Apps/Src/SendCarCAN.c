#include "os.h"
#include "Tasks.h"
#include "bsp.h"
#include "CANbus.h"
#include "CAN_Queue.h"


/**
 * @brief This Task is a basic queue consumer that takes CAN Messages out of the OS Queue and sends them
*/

void Task_SendCarCAN(void *p_arg){
    CANMSG_t msg;

    while (1) {
        CAN_Queue_Pend(&msg);
        CANbus_Send(msg.id,msg.payload,CAN_BLOCKING); //send message
    }
    
}
