/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 */

#include "CANbus.h"
#include "common.h"
#include "config.h"

int generalTest(void) {
    // Tests sending and receiving messages
    uint32_t ids[10] = {0x242, 0x243, 0x244, 0x245, 0x246,
                        0x247, 0x24B, 0x24E, 0x580, CHARGE_ENABLE};
    uint8_t buffer[8];

    CANData_t data;
    data.d = 0x87654321;

    CANPayload_t payload;
    payload.data = data;
    payload.bytes = 4;

    for (int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        CANbus_Send(ids[i], payload, CAN_BLOCKING);
        printf("Sent ID: 0x%x - Success(1)/Failure(0): %d\n", ids[i],
               CANbus_Read(buffer, CAN_BLOCKING));
    }

    exit(0);
}

int Charge_Enable_Test(void) {
    // Tests receiving the MOTOR_DISABLE message

    uint8_t buffer[8];

    while (true) {
        if (CANbus_Read(buffer, CAN_BLOCKING) == SUCCESS) {
            printf("CHARGE_ENABLE Command Received");
        }
    }
}

int main(void) {
    CANbus_Init();
    // generalTest();
    Charge_Enable_Test();
}