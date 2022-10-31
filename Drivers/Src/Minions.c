#include "Minions.h"
#include "os.h"
#include "BSP_GPIO.h"
#include <stdbool.h>
#include "Tasks.h"
#include "Contactors.h"

static OS_MUTEX OutputMutex; //Mutex to lock GPIO writes to input pins

//should be in sync with pin enum
const PinInfo_t PINS_LOOKARR[MINIONPIN_NUM] = {
    {GPIO_Pin_0, PORTA, INPUT},
    {GPIO_Pin_1, PORTA, INPUT},
    {GPIO_Pin_4, PORTA, INPUT},
    {GPIO_Pin_5, PORTA, INPUT},
    {GPIO_Pin_6, PORTA, INPUT},
    {GPIO_Pin_7, PORTA, INPUT},
    {GPIO_Pin_4, PORTB, INPUT},
    {GPIO_Pin_5, PORTB, OUTPUT}
};

void Minion_Init(void){
    OS_ERR err;
    for(uint8_t i = 0; i < MINIONPIN_NUM; i++){
        BSP_GPIO_Init(PINS_LOOKARR[i].port, PINS_LOOKARR[i].pinMask, PINS_LOOKARR[i].direction);
    }
    OSMutexCreate(&OutputMutex, "Minions Output Mutex", &err);
    assertOSError(OS_MINIONS_LOC, err);
}

void Minion_Ignition_Enable_Contactors(void) {
    OS_ERR err;
    Minion_Error_t Merr;

    // Delay for precharge
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    assertOSError(OS_MINIONS_LOC, err);

    Contactors_Enable(MOTOR_CONTACTOR);
    // static bool lastCruiseEnPushed = true; //cruise control edge detector variables
    // static State cruiseEnablePushed = OFF;
    // Main loop
    while (1) {

        //UpdateSwitches();

        // array on/off
        if (Minion_Read_Input(IGN_1, &Merr) == true || Minion_Read_Input(IGN_2, &Merr) == true) {
            Contactors_Enable(ARRAY_CONTACTOR);
            Contactors_Enable(ARRAY_PRECHARGE);
        } else {
            Contactors_Disable(ARRAY_CONTACTOR);
            Contactors_Disable(ARRAY_PRECHARGE);
            //Lights_Set(A_CNCTR,OFF);
            //Display_SetLight(A_CNCTR,OFF);
        }
        assertOSError(OS_MINIONS_LOC, err);

        // motor on/off
        Contactors_Set(MOTOR_CONTACTOR, Minion_Read_Input(IGN_2, &Merr));
        assertOSError(OS_MINIONS_LOC, err);
        //Lights_Set(M_CNCTR,Switches_Read(IGN_2));

        //cruiseEnablePushed = Switches_Read(CRUZ_EN); //read cruise enable switch
        // if (!lastCruiseEnPushed && cruiseEnablePushed) { //Rising Edge detector for updateVelocity thread
        //     UpdateVel_ToggleCruise = true; //updateVelocity thread depends on this
        // }

        //lastCruiseEnPushed = cruiseEnablePushed;


        //OSTimeDlyHMSM(0, 0, 0, READ_SWITCH_PERIOD, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        //assertOSError(OS_SWITCHES_LOC, err);
    }
}


bool Minion_Read_Input(MinionPin_t pin, Minion_Error_t* err){
    if((PINS_LOOKARR[pin].direction == OUTPUT)){ //trying to read from an output pin, can't do that.
        *err = MINION_ERR_YOU_READ_OUTPUT_PIN;
        return false; 
    }   

    return (bool)BSP_GPIO_Read_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinMask);
}

bool Minion_Write_Output(MinionPin_t pin, bool status, Minion_Error_t* mErr){
    CPU_TS timestamp;
    OS_ERR err;

    if(PINS_LOOKARR[pin].direction == OUTPUT){
        OSMutexPend(&OutputMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err); 
        BSP_GPIO_Write_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinMask, status);
        OSMutexPost(&OutputMutex, OS_OPT_POST_NONE, &err);
        assertOSError(OS_MINIONS_LOC, err);
        return true;
    }

    *mErr = MINION_ERR_YOU_WROTE_TO_INPUT_PIN;
    return false;
}