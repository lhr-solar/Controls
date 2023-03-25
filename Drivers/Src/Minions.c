#include "Minions.h"
#include "os.h"
#include "BSP_GPIO.h"
#include <stdbool.h>
#include "Tasks.h"
#include "Contactors.h"

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

    for(uint8_t i = 0; i < MINIONPIN_NUM; i++){
        BSP_GPIO_Init(PINS_LOOKARR[i].port, PINS_LOOKARR[i].pinMask, PINS_LOOKARR[i].direction);
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

    if(PINS_LOOKARR[pin].direction == OUTPUT){
        BSP_GPIO_Write_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinMask, status);
        return true;
    }
    *mErr = MINION_ERR_YOU_WROTE_TO_INPUT_PIN;
    return false;
}