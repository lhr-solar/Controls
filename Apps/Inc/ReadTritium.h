/** 
 * @file ReadTritium.h
 * @brief Reads messages sent by the Tritium motor controller & handles conditional logic based on the message.
 * 
 * Call Motor_RPM_Get() to get the motor velocity in RPM. Call 
 * Motor_Velocity_Get() to get the motor velocity in m/s. Starting the task will
 * automatically forward all messages from MotorCAN to CarCAN, detect and handle motor errors,
 * and forward velocity information to the display.
 * 
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
    /** Hardware over current error */
    kHardwareOverCurrentErr = (1 << 0),
    /** Software over current error */
    kSoftwareOverCurrentErr = (1 << 1),
    /** DC bus over voltage error */
    kDcBusOverVoltErr = (1 << 2),
    /** Hall sensor error */
    kHallSensorErr = (1 << 3),
    /** Watchdog last reset error */
    kWatchdogLastResetErr = (1 << 4),
    /** Config read error */
    kConfigReadErr = (1 << 5),
    /** Under voltage lockout error */
    kUnderVoltageLockoutErr = (1 << 6),
    /** Desaturation fault error */
    kDesatFaultErr = (1 << 7),
    /** Motor over speed error */
    kMotorOverSpeedErr = (1 << 8),
    /** Motor controller fails to restart or initialize */
    kInitFail = (1 << 9),
    /** Motor watchdog trip */
    kMotorWatchdogTrip = (1 << 15),
    /** No error */ 
    kNone = 0x00,
} TritiumErrorCode;

/**
 * @brief Gets the motor velocity in RPM
 * @return float Motor velocity in RPM
 */
float MotorRpmGet();

/**
 * @brief Gets the motor velocity in m/s
 * @return float Motor velocity in m/s
*/
float MotorVelocityGet();

#endif
