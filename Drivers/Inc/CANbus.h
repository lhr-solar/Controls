/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANbus.h
 * @brief
 *
 * @defgroup CANbus
 * @addtogroup CANbus
 * @{
 */

#ifndef CAN_H
#define CAN_H

#include "BSP_CAN.h"

#define CARCAN kCan1  // convenience aliases for the CANBuses
#define MOTORCAN kCan3

/**
 * This enum is used to signify the ID of the message you want to send.
 * It is used internally to index our lookup table (kCanLut.C) and get
 * message-specific fields. For user purposes, it selects the message to send.
 *
 * If changing the order of this enum, make sure to mirror that change in the
 * lookup table, or else the driver will not work properly.
 *
 * If adding new types of CAN messages, add the identifier wherever it fits in
 * (the enum is sorted in ascending order on purpose), and then add an entry to
 * the lookup table.
 */
typedef enum {
    kBpsTrip = 0x002,
    kBpsContactor = 0x102,
    kStateOfCharge = 0x106,
    kSupplementalVoltage = 0x10B,
    kMotorDrive = 0x221,
    kMotorPower = 0x222,
    kMotorReset = 0x223,
    kMotorStatus = 0x241,
    kMcBus = 0x242,
    kVelocity = 0x243,
    kMcPhaseCurrent = 0x244,
    kVoltageVec = 0x245,
    kCurrentVec = 0x246,
    kBackEmf = 0x247,
    kTemperature = 0x24B,
    kOdometerAmpHours = 0x24E,
    kArrayContactorStateChange = 0x24F,
    kSlipSpeed = 0x257,
    kControlMode = 0x580,
    kIoState = 0x581,
    kMaxCanId
} CanId;

/**
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages.
 * @param size Size of message's data. Should be a maximum of eight (in
 * decimal).
 */
typedef struct {
    bool idx_en : 1;
    unsigned int size : 7;
} CanLut;

/**
 * Standard CAN packet
 * @param ID 	CanId value indicating which message we are trying to send
 * @param idx 	If message is part of a sequence of messages (for messages
 * longer than 64 bits), this indicates the index of the message. This is not
 * designed to exceed the 8bit unsigned max value.
 * @param data 	data of the message
 */
typedef struct {
    CanId id;
    uint8_t idx;
    uint8_t data[BSP_CAN_DATA_LENGTH];
} CanData;

// Compatibility macros for deprecated enum
#define CAN_BLOCKING true
#define CAN_NON_BLOCKING false

/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the
 * convenience macros CARCAN and MOTORCAN. CAN2 will not be supported.
 * @param   idWhitelist A list of CAN IDs that we want to receive. If NULL, we
 * will receive all messages.
 * @param   idWhitelistSize The size of the whitelist.
 * @return  ERROR if bus != CAN1 or CAN3, SUCCESS otherwise
 */
ErrorStatus CanBusInit(Can bus, CanId* id_whitelist, uint8_t id_whitelist_size);

/**
 * @brief   Transmits data onto the CANbus. Transmits up to 8 bytes at a time.
 * If more is necessary, please use an IDX message.
 * @param 	CanData 	The data to be transmitted
 * @param 	blocking 	Whether or not this transmission should be a
 * blocking send.
 * @param  	bus			The bus to transmit on. This should be
 * either CARCAN or MOTORCAN.
 * @return  ERROR if data wasn't sent, otherwise it was sent.
 */
ErrorStatus CanBusSend(CanData can_data, bool blocking, Can bus);

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the
 * provided pointers.
 * @param   data 		pointer to where to store the CAN id of the
 * received msg
 * @param   blocking 	Whether or not this read should be a blocking read
 * @param   bus 		The bus to use. This should either be CARCAN or
 * MOTORCAN.
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CanBusRead(CanData* msg_container, bool blocking, Can bus);

#endif

/* @} */
