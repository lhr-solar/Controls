#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>

#define LEN 4
#define MOTOR_DRIVE_ID 0x221

void testCAN1(void) {
    BSP_CAN_Init();

    uint8_t txData[LEN] = {0x00, 0x12, 0x42, 0x5A};
    uint32_t id;

    BSP_CAN_Write(CAN2, 0x201, txData, LEN);

    uint8_t rxData[LEN] = {0};
    uint8_t len = BSP_CAN_Read(CAN2, &id, rxData);
    printf("ID: 0x%x\nData: ", id);
    for (uint8_t i = 0; i < len; i++) {
        printf("0x%x ", rxData[i]);
    }
    printf("\n");
}

void testCAN2(void) {
    BSP_CAN_Init();

    uint8_t data[8] = {0x00, 0x00, 0x00, 0x50,
                0x00, 0x00, 0x00, 0x14};

    while (1) {
        BSP_CAN_Write(CAN2, MOTOR_DRIVE_ID, data, 8);
        // Mimic interrupt frequency without timer sim
        usleep(1000 * 100);
    }

    
}

int main(void) {
    // testCAN1();
    testCAN2();
}
