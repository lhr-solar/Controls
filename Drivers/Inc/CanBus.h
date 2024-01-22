/**
 * @file CANbus.h
 * @brief The CANbus driver is responsible for all incoming and outgoing CAN communication on both 
 * CAN lines. The driver knows of more than a dozen different CAN messages it can send or receive, 
 * and can automatically determine their size and data type.
 */

#ifndef CAN_H
#define CAN_H

#include "BSP_CAN.h"

#define CARCAN CAN_1 //convenience aliases for the CANbuses
#define MOTORCAN CAN_3

/**
 * @brief This enum is used to signify the ID of the message you want to send. 
 * It is used internally to index our lookup table (CANLUT.C) and get message-specific fields.
 * For user purposes, it selects the message to send.
 * 
 * @details
 * If changing the order of this enum, make sure to mirror that change in the lookup table, or
 * else the driver will not work properly. 
 * 
 * If adding new types of CAN messages, add the identifier wherever it fits in 
 * (the enum is sorted in ascending order on purpose), and then add an entry to the lookup table.
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
 * @struct CANLUT_T
 * @brief Struct to use in CAN MSG LUT
 * @param idxEn Whether or not this message is part of a sequence of messages
 * @param size Size of message data in bytes, should not exceed 8
 */
typedef struct {
    bool idx_en : 1;
    unsigned int size : 7;
} CanLut;

/**
 * Standard CAN packet
 * @param ID 	CanId value indicating which message we are trying to send
 * @param idx 	If message is part of a sequence of messages (for messages longer than 64 bits), this indicates the index of the message
 * @param data 	message data
*/
typedef struct {
    CanId id;
    uint8_t idx;
    uint8_t data[BSP_CAN_DATA_LENGTH];
} CanData;

//Compatibility macros for deprecated enum
/**
 * @brief Compatibility macro for deprecated enum
 */
#define CAN_BLOCKING true

/**
 * @brief Compatibility macro for deprecated enum
 */
#define CAN_NON_BLOCKING false

/**
 * @brief   Initializes the CAN system for a given bus
 * @param   bus The bus to initialize. You can either use CAN_1, CAN_3, or the convenience macros CARCAN and MOTORCAN. CAN2 will not be supported
 * @param   idWhitelist A list of CAN IDs that we want to receive. If NULL, we will receive all messages
 * @param   idWhitelistSize The size of the whitelist
 * @return  ERROR if bus isn't CAN1 or CAN3, SUCCESS otherwise
 */
ErrorStatus CanBusInit(Can bus, CanId* id_whitelist, uint8_t id_whitelist_size);

/**
 * @brief   Transmits up to 8 bytes of data onto the CANbus. Use an IDX message to send more data
 * @param 	CanData 	The data to be transmitted
 * @param 	blocking 	Whether or not this transmission should be a blocking send
 * @param  	bus			The bus to transmit on. This should be either CARCAN or MOTORCAN
 * @return  ERROR if data wasn't sent, SUCCESS otherwise
 */
ErrorStatus CanBusSend(CanData can_data, bool blocking, Can bus);

/**
 * @brief   Reads a CAN message from the CAN hardware and returns it to the provided pointers
 * @param   data 		where to store the received message
 * @param   blocking 	Whether or not this read should be blocking
 * @param   bus 		The bus to use, either be CARCAN or MOTORCAN
 * @returns ERROR if read failed, SUCCESS otherwise
 */
ErrorStatus CanBusRead(CanData* msg_container, bool blocking, Can bus);

#endif