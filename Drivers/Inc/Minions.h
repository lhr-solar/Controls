#ifndef MINION_H
#define MINION_H
#include <stdbool.h>
#include "BSP_GPIO.h"

typedef enum{
    //inputs
    REGEN_SW,
    FOR_SW,
    REV_SW,
    CRUZ_EN,
    CRUZ_ST,
    //output
    BRAKELIGHT, 
    MINIONPIN_NUM,
} MinionPin_t; 

typedef struct PinInfo{
    uint16_t pinmask;
    port_t port;
    direction_t direction;        
} PinInfo_t;

PinInfo_t PINS_LOOKARR[MINIONPIN_NUM] = {
    {GPIO_Pin_4, PORTA, INPUT},
    {GPIO_Pin_5, PORTA, INPUT},
    {GPIO_Pin_6, PORTA, INPUT},
    {GPIO_Pin_7, PORTA, INPUT},
    {GPIO_Pin_4, PORTB, INPUT},
    {GPIO_Pin_5, PORTB, OUTPUT}
};

/**
 * @brief Initializes Switches and Brakelight
 * 
 */
void Minion_Init(void);
/**
 * @brief 
 * 
 * @param pin 
 * @return true *NOTE* If output pin is passed, will exit 
 * @return false 
 */
bool Minion_Read_Switch(MinionPin_t pin);
void Minion_Write_Brakelight(State state);

#endif 