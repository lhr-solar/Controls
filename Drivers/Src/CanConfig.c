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
    [kBpsTrip] = {NOIDX, DOUBLE},             /**	   kBpsTrip **/
    [kBpsContactor] = {NOIDX, DOUBLE},        /**	   kBpsContactor        **/
    [kStateOfCharge] = {NOIDX, DOUBLE},       /**     kStateOfCharge      **/
    [kSupplementalVoltage] = {NOIDX, DOUBLE}, /**     kSupplementalVoltage **/
    [kMotorDrive] = {NOIDX, DOUBLE}, /**     kMotorDrive                     **/
    [kMotorPower] = {NOIDX, DOUBLE}, /**     kMotorPower                     **/
    [kMotorReset] = {NOIDX, DOUBLE}, /**     kMotorReset                     **/
    [kMotorStatus] = {NOIDX, DOUBLE}, /**     kMotorStatus **/
    [kMcBus] = {NOIDX, DOUBLE},    /**     kMcBus                          **/
    [kVelocity] = {NOIDX, DOUBLE}, /**     kVelocity                        **/
    [kMcPhaseCurrent] = {NOIDX, DOUBLE}, /**     kMcPhaseCurrent **/
    [kVoltageVec] = {NOIDX, DOUBLE}, /**     kVoltageVec                     **/
    [kCurrentVec] = {NOIDX, DOUBLE}, /**     kCurrentVec                     **/
    [kBackEmf] = {NOIDX, DOUBLE}, /**     kBackEmf                         **/
    [kTemperature] = {NOIDX, DOUBLE},      /**     kTemperature      **/
    [kOdometerAmpHours] = {NOIDX, DOUBLE}, /**     kOdometerAmpHours **/
    [kArrayContactorStateChange] =
        {NOIDX, BYTE}, /**     kArrayContactorStateChange    **/
    [kSlipSpeed] = {NOIDX, DOUBLE},
    [kIoState] = {NOIDX, DOUBLE},   /**     kIoState     **/
    [kControlMode] = {NOIDX, BYTE}, /**     kControlMode **/
};

/**
 * @brief Lists of CAN IDs that we want to receive. Used to initialize the CAN
 * filters for CarCAN and MotorCAN.
 */

CanId car_can_filter_list[NUM_CARCAN_FILTERS] = {
    kBpsTrip,
    kBpsContactor,  // Bit 1 and 0 contain BPS HV Plus/Minus (associated Motor
                    // Controller) Contactor and BPS HV Array Contactor,
                    // respectively
    kStateOfCharge, kSupplementalVoltage};

CanId motor_can_filter_list[NUM_MOTORCAN_FILTERS] = {
    kMcBus,     kVelocity,   kBackEmf, kTemperature, kOdometerAmpHours,
    kSlipSpeed, kMotorStatus};
