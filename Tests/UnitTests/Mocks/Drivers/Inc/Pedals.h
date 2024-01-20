/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_PEDALS
#include_next "Pedals.h"
#else

#ifndef __PEDALS_H
#define __PEDALS_H

#include "BSP_ADC.h"
#include "fff.h"

/**
 * @brief Stuff
 * 
 */
typedef enum 
{
    ACCELERATOR, 
    BRAKE,
    NUMBER_OF_PEDALS
} pedal_t;

DECLARE_FAKE_VOID_FUNC(Pedals_Init);

DECLARE_FAKE_VALUE_FUNC(int8_t, Pedals_Read, pedal_t);

#endif
#endif
