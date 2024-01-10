#ifdef TEST_CONTACTORS
#include_next "Contactors.h" // Include the next instance of the file. 
// If the real version is in the include search paths after the mock one, it will include it here
#else // Mocked Contactors.h
#ifndef __CONTACTORS_H
#define __CONTACTORS_H
#include "fff.h"
#include "common.h"
#include "stm32f4xx_gpio.h"
#include "os.h"

#define CONTACTORS_PORT                                 PORTC
#define ARRAY_PRECHARGE_BYPASS_PIN                      GPIO_Pin_10
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN           GPIO_Pin_12     

#define FOREACH_contactor(contactor) \
        contactor(ARRAY_PRECHARGE_BYPASS_CONTACTOR),  \
        contactor(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR),   \

typedef enum contactor_ENUM {
    FOREACH_contactor(GENERATE_ENUM)
    NUM_CONTACTORS,
}contactor_t;

DECLARE_FAKE_VOID_FUNC(Contactors_Init);
DECLARE_FAKE_VALUE_FUNC(bool, Contactors_Get, contactor_t);
DECLARE_FAKE_VALUE_FUNC(ErrorStatus, Contactors_Set, contactor_t, bool, bool);

#endif
#endif
