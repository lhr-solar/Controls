/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Minions.c
 * @brief
 *
 */
#include "Minions.h"

/* Should be in sync with enum in Minions.h */
const PinInfo kPininfoLut[kNumPins] = {
    {GPIO_Pin_1, kPortA, kInput}, {GPIO_Pin_0, kPortA, kInput},
    {GPIO_Pin_4, kPortA, kInput}, {GPIO_Pin_5, kPortA, kInput},
    {GPIO_Pin_6, kPortA, kInput}, {GPIO_Pin_7, kPortA, kInput},
    {GPIO_Pin_4, kPortB, kInput}, {GPIO_Pin_5, kPortB, kOutput}};

void MinionsInit(void) {
    for (int i = 0; i < kNumPins; i++) {
        BspGpioInit(kPininfoLut[i].port, kPininfoLut[i].pin_mask,
                    kPininfoLut[i].direction);
    }
}

bool MinionsRead(Pin pin) {
    if ((kPininfoLut[pin].direction == kInput)) {
        // Ignition pins are negative logic, special-case them
        if (pin == kIgn1 || pin == kIgn2) {
            return !((bool)BspGpioReadPin(kPininfoLut[pin].port,
                                          kPininfoLut[pin].pin_mask));
        }
        return (bool)BspGpioReadPin(kPininfoLut[pin].port,
                                    kPininfoLut[pin].pin_mask);
    }
    return (bool)BspGpioGetState(kPininfoLut[pin].port,
                                 kPininfoLut[pin].pin_mask);
}

bool MinionsWrite(Pin pin, bool status) {
    if (kPininfoLut[pin].direction == kOutput) {
        BspGpioWritePin(kPininfoLut[pin].port, kPininfoLut[pin].pin_mask,
                        status);
        return true;
    }
    return false;
}
