#include "CarState.h"
#include "UpdateVelocity.h"
#define shift 27
#define first -1
#define second 134

extern const float pedalToPercent[];

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage) {
    return pedalToPercent[pedalPercentage];
}

void Task_UpdateVelocity(void* p_arg) {
    
}

