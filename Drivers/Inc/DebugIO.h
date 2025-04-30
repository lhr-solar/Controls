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

// Macros to generate enums
#define GENERATE_PIN_ENUM(PORT, PIN) P##PORT##PIN,

// Macros to extract port and pin
#define GET_PORT(port, PIN) PORT##port
#define GET_PIN(PORT, PIN) GPIO_Pin_##PIN

// Struct to hold pin information
typedef struct {
    uint32_t port;
    uint32_t pin;
} pinInfo_t;

// Macro to generate pinInfo entries
#define GENERATE_PIN_INFO(PORT, PIN) { .port = GET_PORT(PORT, PIN), .pin = GET_PIN(PORT, PIN) },

// Use enum to index into debugGPIOLUT
typedef enum {
    FOREACH_PIN(GENERATE_PIN_ENUM)
    NUM_PINS // Total number of pins
} pinIndex_t;

// Array of pinInfo structs
static const pinInfo_t debugGPIOLUT[NUM_PINS] = {
    FOREACH_PIN(GENERATE_PIN_INFO)
};

void DebugIO_Init(void);
void DebugIO_Write(pinIndex_t pin, bool state);
void DebugIO_Toggle(pinIndex_t pin);

#endif // DEBUGIO_H