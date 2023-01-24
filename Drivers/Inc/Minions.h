#ifndef MINION_H
#define MINION_H
#include <stdbool.h>
#include "BSP_GPIO.h"

// used to index into lookup table
// if changed, PINS_LOOKARR should be changed in Minions.c
#define FOREACH_MinionPin(PIN) \
        PIN(IGN_1)   \
        PIN(IGN_2)  \
        PIN(REGEN_SW)   \
        PIN(FOR_SW)  \
        PIN(REV_SW)  \
        PIN(CRUZ_EN)  \
        PIN(CRUZ_ST)  \
        PIN(BRAKELIGHT)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum MINIONPIN_ENUM {
    FOREACH_MinionPin(GENERATE_ENUM)
    MINIONPIN_NUM,
}MinionPin_t;


//errors bc ur bad
typedef enum{
    MINION_ERR_NONE = 0,
    MINION_ERR_YOU_READ_OUTPUT_PIN,
    MINION_ERR_YOU_WROTE_TO_INPUT_PIN,
} Minion_Error_t;


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
bool Minion_Read_Input(MinionPin_t pin, Minion_Error_t* err);

/**
 * @brief Writes given status to a specified output pin. Locks writing to all output pins
 * 
 * @param pin specific pin to be written to 
 * @param status state of pin (0 or 1)
 * @return false if pin is not an output pin, true if it is
 */
bool Minion_Write_Output(MinionPin_t pin, bool status, Minion_Error_t* mErr);


#endif 