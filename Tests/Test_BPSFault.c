/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "CANConfig.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "ReadCarCAN.h"
#include "Tasks.h"

/*
 * NOTE: This test must be run with car CAN in loopback mode
 * TODO: automate this, either with arguments to BSP or #define
 */

static OS_TCB Task1_TCB;
#define STACK_SIZE 256
static CPU_STK Task1_Stk[STACK_SIZE];

#define CARCAN_FILTER_SIZE (sizeof carCANFilterList / sizeof(CANId_t))

void Task1() {
  OS_ERR err;

  CPU_Init();
  OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
  CANbus_Init(CARCAN, carCANFilterList, CARCAN_FILTER_SIZE);
  Contactors_Init();

  // Send a BPS trip
  CANDATA_t data = {.ID = BPS_TRIP, .idx = 0, .data = {1}};
  CANbus_Send(data, true, CARCAN);
  OSTaskDel(NULL, &err);
}

int main() {
  OS_ERR err;
  OSInit(&err);
  assertOSError(OS_MAIN_LOC, err);
  TaskSwHook_Init();

  OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);
  OSTaskCreate(
      (OS_TCB*)&Task1_TCB, (CPU_CHAR*)"Task1", (OS_TASK_PTR)Task1, (void*)NULL,
      (OS_PRIO)4, (CPU_STK*)Task1_Stk, (CPU_STK_SIZE)STACK_SIZE / 10,
      (CPU_STK_SIZE)STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)NULL, (void*)NULL,
      (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK), (OS_ERR*)&err);
  assertOSError(OS_MAIN_LOC, err);

  OSTaskCreate(
      (OS_TCB*)&FaultState_TCB, (CPU_CHAR*)"Fault State",
      (OS_TASK_PTR)Task_FaultState, (void*)NULL, (OS_PRIO)TASK_FAULT_STATE_PRIO,
      (CPU_STK*)FaultState_Stk, (CPU_STK_SIZE)STACK_SIZE / 10,
      (CPU_STK_SIZE)STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)NULL, (void*)NULL,
      (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK), (OS_ERR*)&err);
  assertOSError(OS_MAIN_LOC, err);

  OSTaskCreate((OS_TCB*)&ReadCarCAN_TCB, (CPU_CHAR*)"ReadCarCAN",
               (OS_TASK_PTR)Task_ReadCarCAN, (void*)NULL,
               (OS_PRIO)TASK_READ_CAR_CAN_PRIO, (CPU_STK*)ReadCarCAN_Stk,
               (CPU_STK_SIZE)STACK_SIZE / 10, (CPU_STK_SIZE)STACK_SIZE,
               (OS_MSG_QTY)0, (OS_TICK)NULL, (void*)NULL,
               (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK),
               (OS_ERR*)&err);
  assertOSError(OS_MAIN_LOC, err);

  OSStart(&err);
  assertOSError(OS_MAIN_LOC, err);
}
