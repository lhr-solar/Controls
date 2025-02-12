/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Minions.c
 * @brief 
 * 
 */
#include "Minions.h"

/* Should be in sync with enum in Minions.h */
const pinInfo_t PININFO_LUT[NUM_PINS] = {
    {GPIO_Pin_1, PORTA, INPUT},
    {GPIO_Pin_0, PORTA, INPUT},
    {GPIO_Pin_4, PORTA, INPUT},
    {GPIO_Pin_5, PORTA, INPUT},
    {GPIO_Pin_6, PORTA, INPUT},
    {GPIO_Pin_7, PORTA, INPUT},
    {GPIO_Pin_4, PORTB, INPUT},
    {GPIO_Pin_5, PORTB, OUTPUT}
};

void Minions_Init(void){
    for(uint8_t i = 0; i < NUM_PINS; i++){
        BSP_GPIO_Init(PININFO_LUT[i].port, PININFO_LUT[i].pinMask, PININFO_LUT[i].direction, false);
    }
}

bool Minions_Read(pin_t pin){
    if((PININFO_LUT[pin].direction == INPUT)){
        if (pin == IGN_1 || pin == IGN_2) {
            return !((bool) BSP_GPIO_Read_Pin(PININFO_LUT[pin].port, PININFO_LUT[pin].pinMask));
        } else {
            return (bool) BSP_GPIO_Read_Pin(PININFO_LUT[pin].port, PININFO_LUT[pin].pinMask);
        }
    } else{
        return (bool)BSP_GPIO_Get_State(PININFO_LUT[pin].port, PININFO_LUT[pin].pinMask);
    }
}

bool Minions_Write(pin_t pin, bool status){
    if(PININFO_LUT[pin].direction == OUTPUT){
        BSP_GPIO_Write_Pin(PININFO_LUT[pin].port, PININFO_LUT[pin].pinMask, status);
        return true;
    }
    return false;
}
