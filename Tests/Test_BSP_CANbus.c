/**
 * Test file for library to interact with CAN
 * 
 * Run this test in conjunction with the simulator.
 * 
 * testCAN1 will simply write a message to the CAN1 line
 * that will then be displayed by the GUI in the CAN seciton.
 * 
 * testCAN2 will periodically send the MOTOR DRIVE command to
 * CAN2 (the line connected to the motor controller). It writes
 * a desired velocity, which will be displayed on the GUI. 
 * If the program terminates, the motor simulator will go into
 * neutral, meaning it will stop changing velocity.
 */ 

#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>

#define LEN 4
#define MOTOR_DRIVE_ID 0x221

void testCAN1(void) {
    // Just to mimic what will actually happen
    BSP_CAN_Init(CAN_1);
    BSP_CAN_Init(CAN_2);

    uint8_t txData[LEN] = {0x00, 0x12, 0x42, 0x5A};
    uint32_t id;

    BSP_CAN_Write(CAN_1, 0x201, txData, LEN);

    uint8_t rxData[LEN] = {0};
    uint8_t len = BSP_CAN_Read(CAN_1, &id, rxData);
    printf("ID: 0x%x\nData: ", id);
    for (uint8_t i = 0; i < len; i++) {
        printf("0x%x ", rxData[i]);
    }
    printf("\n");
}

void testCAN2(void) {
    // Just to mimic what will actually happen
    BSP_CAN_Init(CAN_1);
    BSP_CAN_Init(CAN_2);

    uint8_t data[8] = {0x00, 0x00, 0x00, 0x50,
                0x00, 0x00, 0x00, 0x14};

    while (1) {
        BSP_CAN_Write(CAN_2, MOTOR_DRIVE_ID, data, 8);
        // Mimic interrupt frequency without timer sim
        usleep(1000 * 100);
    }

    
}

void testCAN3(void) {
    // Just to mimic what will actually happen
    BSP_CAN_Init(CAN_1);
    BSP_CAN_Init(CAN_2);
    
    uint32_t id;

    uint8_t rxData[LEN] = {0};
    uint8_t len = BSP_CAN_Read(CAN_1, &id, rxData);
    
    if(len!=0){
        printf("ID: 0x%x\nData: ", id);
        for (uint8_t i = 0; i < len; i++) {
            printf("0x%x ", rxData[i]);
        }
        printf("\n");
    }
}

int main(void) {
    testCAN1();
    // testCAN2();
    //testCAN3();
}
