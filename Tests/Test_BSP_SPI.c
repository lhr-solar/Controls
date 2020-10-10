#include "BSP_SPI.h"

int main(void) {
    BSP_SPI_Init();
    uint8_t txData[] = {0x41, 0x12, 0x00};
    uint8_t txLen = 3;
    BSP_SPI_Write(txData, txLen);
    uint8_t rxData[1];
    uint8_t rxLen = 1;
    usleep(1000000);
    BSP_SPI_Read(rxData, rxLen);
    printf("Received Data: %x\n\r", rxData[0]);
    return 0;
}
