/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 */

#include "CAN_Queue.h"
#include "CANbus.h"
#include "Tasks.h"
#include "stm32f4xx.h"

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

#define STACK_SIZE 128

void Task1(void *p_arg) {
  CPU_Init();
  // OS_CPU_SysTickInit();
  OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
  OS_ERR err;
  // CPU_TS ts;
  CAN_Queue_Init();
  CANbus_Init(CARCAN);

  // spawn can send task
  OSTaskCreate(
      (OS_TCB *)&SendCarCAN_TCB, (CPU_CHAR *)"SendCarCan",
      (OS_TASK_PTR)Task_SendCarCAN, (void *)NULL, (OS_PRIO)3,
      (CPU_STK *)SendCarCAN_Stk, (CPU_STK_SIZE)STACK_SIZE / 10,
      (CPU_STK_SIZE)STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)NULL, (void *)NULL,
      (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK), (OS_ERR *)&err);

  CANDATA_t msg;
  msg.ID = MOTOR_STATUS;
  msg.idx = 0;
  msg.data[0] = 0x12;
  msg.data[1] = 0x34;
  msg.data[2] = 0x56;
  msg.data[3] = 0x78;
  msg.data[4] = 0x9A;
  msg.data[5] = 0xBC;
  msg.data[6] = 0xDE;
  msg.data[7] = 0xF0;
  while (1) {
    (msg.ID)++;
    if (msg.ID > 0x24F) {
      msg.ID = MOTOR_STATUS;
    }
    CAN_Queue_Post(msg);
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}

int main(void) {  // startup OS stuff, spawn test task

  OS_ERR err;
  OSInit(&err);
  if (err != OS_ERR_NONE) {
    printf("OS error code %d\n", err);
  }
  OSTaskCreate((OS_TCB *)&Task1_TCB, (CPU_CHAR *)"Task1", (OS_TASK_PTR)Task1,
               (void *)NULL, (OS_PRIO)4, (CPU_STK *)Task1_Stk,
               (CPU_STK_SIZE)STACK_SIZE / 10, (CPU_STK_SIZE)STACK_SIZE,
               (OS_MSG_QTY)0, (OS_TICK)NULL, (void *)NULL,
               (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK),
               (OS_ERR *)&err);

  if (err != OS_ERR_NONE) {
    printf("Task1 error code %d\n", err);
  }
  OSStart(&err);
  if (err != OS_ERR_NONE) {
    printf("OS error code %d\n", err);
  }
  return 0;
}
