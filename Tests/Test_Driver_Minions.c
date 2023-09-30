/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 */

#include <bsp.h>

#include "Minions.h"
#include "Tasks.h"
#include "common.h"
#include "config.h"
#include "os.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

#define tof(b) (b ? "on" : "off")

void Task1(void *arg) {
  CPU_Init();
  OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

  BSP_UART_Init(UART_2);
  Minion_Init();

  OS_ERR err;
  Minion_Error_t mErr;
  bool brake_on = false;
  bool ign1, ign2, regen_sw, for_sw, rev_sw, cruz_en, cruz_st;

  for (;;) {
    Minion_Write_Output(BRAKELIGHT, brake_on, &mErr);
    ign1 = !Minion_Read_Pin(IGN_1, &mErr);
    ign2 = !Minion_Read_Pin(IGN_2, &mErr);
    regen_sw = Minion_Read_Pin(REGEN_SW, &mErr);
    for_sw = Minion_Read_Pin(FOR_SW, &mErr);
    rev_sw = Minion_Read_Pin(REV_SW, &mErr);
    cruz_en = Minion_Read_Pin(CRUZ_EN, &mErr);
    cruz_st = Minion_Read_Pin(CRUZ_ST, &mErr);
    printf(
        "--------------------\r\nign1: %s\r\nign2: %s\r\n"
        "regen_sw: %s\r\nfor_sw: %s\r\nrev_sw: %s\r\n"
        "cruz_en: %s\r\ncruz_st: %s\r\n--------------------\r\n\r\n",
        tof(ign1), tof(ign2), tof(regen_sw), tof(for_sw), tof(rev_sw),
        tof(cruz_en), tof(cruz_st));

    brake_on = !brake_on;
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
  }
};

int main(void) {
  OS_ERR err;
  OSInit(&err);

  // create tester thread
  OSTaskCreate((OS_TCB *)&Task1TCB, (CPU_CHAR *)"Task 1", (OS_TASK_PTR)Task1,
               (void *)NULL, (OS_PRIO)13, (CPU_STK *)Task1Stk,
               (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
               (CPU_STK_SIZE)DEFAULT_STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)NULL,
               (void *)NULL, (OS_OPT)(OS_OPT_TASK_STK_CLR), (OS_ERR *)&err);
  assertOSError(OS_MAIN_LOC, err);

  OSStart(&err);
}
