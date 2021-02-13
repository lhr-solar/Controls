#include <stdint.h>
//Header to typedef any structs needed to hold states before declaring in Tasks.c and externing in Tasks.h
typedef struct SwitchStates { //Holds all Switch States, updated by ReadSwitches RTOS task
    uint8_t CRUZ_EN : 1;
    uint8_t CRUZ_SW : 1;
    uint8_t HZD_SQ : 1;
    uint8_t FR_SW : 1;
    uint8_t HEADLIGHT_SW : 1;
    uint8_t LEFT_SW : 1;
    uint8_t RIGHT_SW : 1;
    uint8_t REGEN_SW : 1;
    uint8_t IGN_1 : 1;
    uint8_t IGN_2 : 1;
    uint8_t REV_SW : 1;
} SwitchStates;

typedef struct LightStates {
    uint8_t A_CNCTR  : 1;
    uint8_t M_CNCTR : 1;
    uint8_t CTRL_FAULT : 1;
    uint8_t LEFT_BLINK : 1;
    uint8_t RIGHT_BLINK : 1;
    uint8_t Headlight_ON : 1;
    uint8_t BPS_FAULT : 1;
    uint8_t BPS_PWR : 1;
    uint8_t  BrakeLight : 1;
    uint8_t RSVD_LED : 1;
} LightStates;