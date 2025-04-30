#ifndef DEBUGIO_H
#define DEBUGIO_H
#include "BSP_GPIO.h"
#include "daybreak_pins.h"

// Define the list of pins
#define FOREACH_PIN(PIN) \
    PIN(C, 9)           \
    PIN(C, 10)          \
    PIN(C, 11)          \
    PIN(C, 12)          \
    PIN(C, 13)          \
    PIN(C, 14)          \
    PIN(A, 8)           \
    PIN(A, 15)          \
    PIN(B,  7)          \
    PIN(D,  2)          \

// Generates the enum for each pin entry (ie PC17)
#define GENERATE_PIN_ENUM(PORT, PIN) P##PORT##PIN,

#define GET_PORT(port, PIN) PORT##port
#define GET_PIN(PORT, PIN) GPIO_Pin_##PIN

// Struct to hold pin information
typedef struct {
    uint32_t port;
    uint32_t pin;
} pinInfo_t;

// Generates entries in the debugGPIOLUT array
#define GENERATE_PIN_INFO(PORT, PIN) { .port = GET_PORT(PORT, PIN), .pin = GET_PIN(PORT, PIN) },

// Use enum to index into debugGPIOLUT
typedef enum {
    FOREACH_PIN(GENERATE_PIN_ENUM)
    NUM_DEBUG_PINS // Total number of pins
} pinIndex_t;

// Array of pinInfo structs
static const pinInfo_t debugGPIOLUT[NUM_DEBUG_PINS] = {
    FOREACH_PIN(GENERATE_PIN_INFO)
};

/**
 * @brief   Inits all debug pins
 * @param   None
 * @return  None
 */ 
void DebugIO_Init(void);

/**
 * @brief   Writes to a debug pin
 * @param   pin the pin to write to (PC17)
 * @return  None
 */ 
void DebugIO_Write(pinIndex_t pin, bool state);

/**
 * @brief   Toggles a debug pin
 * @param   pin the pin to write to (PC17)
 * @return  None
 */ 
void DebugIO_Toggle(pinIndex_t pin);

#endif // DEBUGIO_H