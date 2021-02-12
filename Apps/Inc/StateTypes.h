#include <stdint.h>
//Header to typedef any structs needed to hold states before declaring in Tasks.c and externing in Tasks.h

//ALL OF THESE NEED TO BE SET TO 0 AT INITIALIZATION
typedef struct SwitchStates { //Holds all Switch States, updated by ReadSwitches RTOS task
    uint8_t CRUZ_SW;
    uint8_t CRUZ_EN;
    uint8_t HZD_SQ;
    uint8_t FR_SW;
    uint8_t HEADLIGHT_SW;
    uint8_t LEFT_SW;
    uint8_t RIGHT_SW;
    uint8_t REGEN_SW;
    uint8_t IGN_1;
    uint8_t IGN_2;
    uint8_t REV_SW;
} SwitchStates;

typedef struct LightStates {
    uint8_t A_CNCTR ;
    uint8_t M_CNCTR;
    uint8_t CTRL_FAULT;
    uint8_t LEFT_BLINK;
    uint8_t RIGHT_BLINK;
    uint8_t Headlight_ON;
    uint8_t BPS_FAULT;
    uint8_t BPS_PWR;
    uint8_t  BrakeLight;
    uint8_t RSVD_LED;
} LightStates;

typedef struct IgnitionStates {
    uint8_t IGN1;
    uint8_t IGN2;
} IgnitionStates