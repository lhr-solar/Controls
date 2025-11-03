#include "ReadCAN.h"

// Initialize can recieve semaphores and carcan semaphore.
// Only call this function in a task
void ReadCAN_Init(){

}

void Task_ReadCAN(void *p_arg) {

    while (1){
        // Read a message from the CAN bus
        CANDATA_t can_msg;
        ErrorStatus status = CANbus_Read(&can_msg, true, CARCAN);
        // iterate through the list of can_recv_entry, and post data to that entry if the ID matches
        
    }

}