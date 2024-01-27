/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANConfig.c
 * @brief
 *
 */

#include "CanConfig.h"

#define BYTE 1
#define HALFWORD 2
#define WORD 4
#define DOUBLE 8
#define NOIDX false
#define IDX true

/**
 * @brief Lookup table to simplify user-defined packet structs. Contains
 * metadata fields that are always the same for every message of a given ID.
 *        Indexed by CanId values. Any changes or additions must be made in
 * parallel with changes made to the CANID_t enum in CanBus.h
 */
const CanLut kCanLut[kMaxCanId] = {
    [kBpsTrip] = {NOIDX, DOUBLE},
    [kBpsContactor] = {NOIDX, DOUBLE},
    [kStateOfCharge] = {NOIDX, DOUBLE},
    [kSupplementalVoltage] = {NOIDX, DOUBLE},
    [kMotorDrive] = {NOIDX, DOUBLE},
    [kMotorPower] = {NOIDX, DOUBLE},
    [kMotorReset] = {NOIDX, DOUBLE},
    [kMotorStatus] = {NOIDX, DOUBLE},
    [kMcBus] = {NOIDX, DOUBLE},
    [kVelocity] = {NOIDX, DOUBLE},
    [kMcPhaseCurrent] = {NOIDX, DOUBLE},
    [kVoltageVec] = {NOIDX, DOUBLE},
    [kCurrentVec] = {NOIDX, DOUBLE},
    [kBackEmf] = {NOIDX, DOUBLE},
    [kTemperature] = {NOIDX, DOUBLE},
    [kOdometerAmpHours] = {NOIDX, DOUBLE},
    [kArrayContactorStateChange] = {NOIDX, BYTE},
    [kSlipSpeed] = {NOIDX, DOUBLE},
    [kIoState] = {NOIDX, DOUBLE},
    [kControlMode] = {NOIDX, BYTE},
};

/**
 * @brief Lists of CAN IDs that we want to receive. Used to initialize the CAN
 * filters for CarCAN and MotorCAN.
 */

CanId car_can_filter_list[NUM_CARCAN_FILTERS] = {
    kBpsTrip,
    kBpsContactor,  // Bit 1 and 0 contain BPS HV Plus-Minus (associated Motor
                    // Controller) Contactor and BPS HV Array Contactor,
                    // respectively
    kStateOfCharge, kSupplementalVoltage};

CanId motor_can_filter_list[NUM_MOTORCAN_FILTERS] = {
    kMcBus,     kVelocity,   kBackEmf, kTemperature, kOdometerAmpHours,
    kSlipSpeed, kMotorStatus};
