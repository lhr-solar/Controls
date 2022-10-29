#include "Minions.h"
#include "os.h"
#include "BSP_GPIO.h"
#include <stdbool.h>
#include "Tasks.h"

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

bool Minion_Read_Input(MinionPin_t pin){
    if((PINS_LOOKARR[pin].direction == OUTPUT)){ //trying to read from an output pin, can't do that.
        return false; 
    }   

    return (bool)BSP_GPIO_Read_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinMask);
}

bool Minion_Write_Output(MinionPin_t pin, bool status){
    CPU_TS timestamp;
    OS_ERR err;

    if(PINS_LOOKARR[pin].direction == OUTPUT){
        OSMutexPend(&OutputMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err); 
        BSP_GPIO_Write_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinMask, status);
        OSMutexPost(&OutputMutex, OS_OPT_POST_NONE, &err);
        assertOSError(OS_MINIONS_LOC, err);
        return true;
    }
    return false;
}