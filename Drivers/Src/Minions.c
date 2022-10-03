#include "Minions.h"
#include "os.h"
#include "BSP_GPIO.h"

static OS_MUTEX CommMutex; //Mutex to lock SPI lines
// Stores switch states:
// IGN_1 | IGN_2 | HZD_SW | REGEN_SW | RIGHT_SW | LEFT_SW | Headlight_SW | FOR_SW | REV_SW | CRUZ_EN | CRUZ_ST
static uint16_t switchStatesBitmap = 0;


void Minion_Init(void){
    for(uint8_t i = 0; i < MINIONPIN_NUM; i++){
        BSP_GPIO_Init(PINS_LOOKARR[i].pinmask, PINS_LOOKARR[i].port, PINS_LOOKARR[i].direction);
    }
}

bool Minion_Read_Switch(MinionPin_t pin){
    if(pin == BRAKELIGHT){
        return false; //trying to read from an output pin, can't do that.
    }    

    return (bool)BSP_GPIO_Read_Pin(PINS_LOOKARR[pin].port, PINS_LOOKARR[pin].pinmask);
}

void Minion_Write_Brakelight(State state){
    BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinmask, state);
}