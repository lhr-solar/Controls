//Header to typedef any structs needed to hold states before declaring in Tasks.c and externing in Tasks.h
typedef struct SwitchStates { //Holds all Switch States, updated by ReadSwitches RTOS task
    unsigned int CRUZ_SW : 1;
    unsigned int CRUZ_EN : 1;
    unsigned int HZD_SQ : 1;
    unsigned int FR_SW : 1;
    unsigned int HEADLIGHT_SW : 1;
    unsigned int LEFT_SW : 1;
    unsigned int RIGHT_SW : 1;
    unsigned int REGEN_SW : 1;
    unsigned int IGN_1 : 1;
    unsigned int IGN2 : 1;
    unsigned int REV_SW : 1;
} SwitchStates;