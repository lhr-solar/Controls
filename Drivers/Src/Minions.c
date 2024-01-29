/**
 * @file Minions.c
 * @details
 * # Implementation Details
 * All of the switches and lights that are controllable via software are
 * connected through GPIO. These include:
 * - Ignition Switch 1 & 2 (ARR_ON and MOTOR_ON)
 * - Regen Enable Button
 * - Gear Switch (Forward and Reverse) (note: if both Forward and Reverse are
 * off, the gear switch is assumed to be in Neutral state)
 * - Cruise Enable Button
 * - Cruise Set Button
 * - Brakelight
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
