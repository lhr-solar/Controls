#ifdef TEST_READCARCAN
#include_next "ReadCarCAN.h" // Include the next instance of the file. 
// If the real version is in the include search paths after the mock one, it will include it here
#else // Mocked Contactors.h
#ifndef MINIONS_H
#define MINIONS_H
#include "fff.h"
#include "common.h"
#include <stdbool.h>
#include "BSP_GPIO.h"


// used to index into lookup table
// if changed, PINS_LOOKARR should be changed in Minions.c
#define FOREACH_PIN(PIN) \
        PIN(IGN_1),   \
        PIN(IGN_2),  \
        PIN(REGEN_SW),   \
        PIN(FOR_SW),  \
        PIN(REV_SW),  \
        PIN(CRUZ_EN),  \
        PIN(CRUZ_ST),  \
        PIN(BRAKELIGHT),  \

typedef enum MINIONPIN_ENUM {
    FOREACH_PIN(GENERATE_ENUM)
    NUM_PINS,
} pin_t;

typedef struct {
    uint16_t pinMask;
    port_t port;
    direction_t direction;
} pinInfo_t;

DECLARE_FAKE_VOID_FUNC(Minions_Init);

DECLARE_FAKE_VALUE_FUNC(bool, Minions_Read, pin_t);

#endif
#endif