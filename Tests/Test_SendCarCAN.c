#include "Tasks.h"
#include "CANbus.h"

void Task1(void *p_arg){
    OS_ERR err;
    CPU_TS ts;
    OSQCreate(); //initializes CAN Send queue
    OSQPost();//populate CAN SEND Queue with test msgs
    //spawn CAN send task
    //Use CANRead to read back the messages to see if they match properly
}
void main(void){ //startup OS stuff, spawn test task
    static OS_TCB Task1_TCB;
    static CPU_STK Task1_Stk[128];
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    INIT_TASK(Task1,3,NULL,err);
     if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}