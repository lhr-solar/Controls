#include "common.h"
#include "config.h"
#include <bsp.h>

int main(void) {
    BSP_CAN_Init();

    uint8_t d[4] = {0x00, 0x01, 0x02, 0x03};
    BSP_CAN1_Write(0xffffffff, d, 4);
}
