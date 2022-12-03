#ifndef MINION_H
#define MINION_H
#include <stdbool.h>
#include "BSP_GPIO.h"


typedef enum{
    MINION_ERR_NONE = 0,
    MINION_ERR_WROTE_INPUT,
} Minion_Error_t;

//used to index into lookup table
//if changed, PINS_LOOKARR should be changed in Minions.c
typedef enum{
    //inputs
    IGN_1,
    IGN_2,
    REGEN_SW,
    FOR_SW,
    REV_SW,
    CRUZ_EN,
    CRUZ_ST,
    //output
    BRAKELIGHT, 
    //num of pins
    MINIONPIN_NUM,
} MinionPin_t; 

typedef struct PinInfo{
    uint16_t pinMask;
    port_t port;
    direction_t direction; //0 for input, 1 for output     
} PinInfo_t;


/**
 * @brief Initializes input switches, output pins, and output mutex
 * 
 */
void Minion_Init(void);

/**
 * @brief Reads current state of specified input pin
 * 
 * @param pin specific pin to be read 
 * @return true
 * @return false *NOTE* If output pin is passed, will exit 
 */
bool Minion_Read_Pin(MinionPin_t pin, Minion_Error_t* err);

/**
 * @brief Writes given status to a specified output pin. Locks writing to all output pins
 * 
 * @param pin specific pin to be written to 
 * @param status state of pin (0 or 1)
 * @return false if pin is not an output pin, true if it is
 */
bool Minion_Write_Output(MinionPin_t pin, bool status, Minion_Error_t* mErr);


#endif 