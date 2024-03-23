/**
 * Test file for BSP_CAN
 * 
 * Use test_send to send and test_read to read
 * 
 * Parameter for the test is the bus you want to test on
*/

#include "BSP_CAN.h"

void test_send(CAN_t bus);
void test_read(CAN_t bus);

int main(void) {
    // test_send(CAN_1);
    test_read(CAN_1);
    while(1) { }
}

void test_send(CAN_t bus) {
    BSP_CAN_Init(bus, NULL, NULL, NULL, 0);

    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    BSP_CAN_Write(bus, 0x001, data, 8);
}

void test_read(CAN_t bus) {
    BSP_CAN_Init(bus, NULL, NULL, NULL, 0);
    BSP_UART_Init(UART_2);

    uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    BSP_CAN_Write(bus, 0x001, tx_data, 8);

    uint32_t rx_id;
    uint8_t rx_data[8];

    for(int i = 0; i < 999999; i++) {} // delay for hardware of read hardware to update

    BSP_CAN_Read(bus, &rx_id, rx_data);
    
    printf("%ld\n\r", rx_id);
    for(int i = 0; i < 8; i++) {
        printf("%x ", rx_data[i]);
    }
    printf("\n\r");
}
