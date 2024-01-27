/* Copyright (c) 2021 UT Longhorn Racing Solar
 * @file ReadTritium.h
 * @brief
 *
 * @defgroup ReadTritium
 * @addtogroup ReadTritium
 * @{
 */

#ifndef READ_TRITIUM_H
#define READ_TRITIUM_H

/**
 * Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate error
 * messages as needed based on bits
 *
 */
typedef enum {
    kHardwareOverCurrentErr = (1 << 0),
    kSoftwareOverCurrentErr = (1 << 1),
    kDcBusOverVoltErr = (1 << 2),
    kHallSensorErr = (1 << 3),
    kWatchdogLastResetErr = (1 << 4),
    kConfigReadErr = (1 << 5),
    kUnderVoltageLockoutErr = (1 << 6),
    kDesatFaultErr = (1 << 7),
    kMotorOverSpeedErr = (1 << 8),
    kInitFail = (1 << 9),  // motor controller fails to restart or initialize
    kMotorWatchdogTrip = (1 << 15),
    kNone = 0x00,
} TritiumErrorCode;

/**
 * Task Prototype
 */
void TaskReadTritium(void* p_arg);

float MotorRpmGet();
float MotorVelocityGet();

#endif

/* @} */
