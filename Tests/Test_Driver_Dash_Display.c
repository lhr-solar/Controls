#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"
#include "Display.h" 
// #include "bsp.h"
// #include "Contactors.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "ReadTritium.h"
#include "Dash.h"
#include "StatusLeds.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void delay(){
    OS_ERR e;
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &e);
}

void testTriStateComp(UpdateDisplayError_t(*function)(TriState_t)){
    function(STATE_0); // DISP_DISABLED & DISP_NEUTRAL
    
    delay();
    function(STATE_1); // DISP_ENABLED & DISP_FORWARD
    
    delay();
    function(STATE_2); // DISP_ACTIVE & DISP_REVERSE
    
    delay();
    function(STATE_0);
    
    delay();
}

void Task1(void *arg)
{   

    CPU_Init();
    Display_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    Status_Leds_Init();
    dashInit();
    dashboard_t dash = {0, 0, 0, 0, 0, 0, 0};

    UpdateDisplay_Init();
    
     OS_ERR e;

    OSTaskCreate(
        (OS_TCB *)&UpdateDisplay_TCB,
        (CPU_CHAR *)"UpdateDisplay_TCB",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)UpdateDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&e);
    assertOSError(e);

    while(1){
        for (uint8_t i = 0; i < NUM_DASH_PINS; i++) {
            switch(i) {
                  case(BPS):
                      dash.BPS_HZD = getDash(i);
                      break;
  
                  case(BRAKE_LED):
                      dash.BRAKE = getDash(i);
                      break;
  
                  case(GEAR):
                      dash.GEAR = getDash(i);
                        UpdateDisplay_SetGear(dash.GEAR);
                        delay();
  
                  case(HBT):
                      dash.HBT = getDash(i);
                      break;
  
                  case(CRUZ_SET):
                      dash.CRUZ_SET = getDash(i);
                      break;
  
                  case(CRUZ_EN):
                      dash.CRUZ_EN = getDash(i);
                      break;

                    case(IGN):
                    dash.IGN = getDash(i);
                    if(dash.IGN == OFF_IGN){
                        UpdateDisplay_SetArray(false);
                        UpdateDisplay_SetMotor(false);
                    }
                    if(dash.IGN == IGN_1){
                        UpdateDisplay_SetArray(true);
                        UpdateDisplay_SetMotor(false);
                    }
                    if(dash.IGN == IGN_2){
                        UpdateDisplay_SetArray(true);
                        UpdateDisplay_SetMotor(true);
                    }  
                  default:
                      break;
          }   
  
          if(dash.CRUZ_EN){
              Status_Leds_Toggle(CRUISE_IND_LED);
          }
          if(dash.CRUZ_SET){
              Status_Leds_Toggle(CRUISE_IND_LED);
          }
  
          if(dash.GEAR == FWD){
              Status_Leds_Write(BPS_FAULT_LED, true);
          } else if(dash.GEAR == REV){
              Status_Leds_Write(CONTROLS_FAULT_LED, true);
          } else {
              Status_Leds_Write(BPS_FAULT_LED, false);
              Status_Leds_Write(CONTROLS_FAULT_LED, false);
          }
      }
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)12,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);

    OSStart(&err);
}