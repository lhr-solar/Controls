/**
 * @file
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @brief Test file for library to interact with the pedal driver file
 *
 * Run this test in conjunction with the simulator
 * GUI. As you move the accelerator and brake on the GUI, the respective
 * pressed/slided percentage will change from '0' to '100' on the terminal and
 * display to show that sliding the pedals is read by the BSP
 */

#include <bsp.h>

#include "Pedals.h"
#include "common.h"
#include "config.h"

int main() {
    Pedals_Init();

    while (1) {
        printf("Accelerator: %5.1d%%\tBrake: %5.1d%%\r",
               Pedals_Read(ACCELERATOR), Pedals_Read(BRAKE));
    }
}
