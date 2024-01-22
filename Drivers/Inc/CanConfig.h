/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file CANConfig.h
 * @brief 
 * 
 */
#ifndef CAN_CONFIG
#define CAN_CONFIG

#include "CanBus.h"

/**
 * Filter Lists for CarCAN and MotorCAN
 */
#define NUM_CARCAN_FILTERS 4
#define NUM_MOTORCAN_FILTERS 7
extern CanId car_can_filter_list[NUM_CARCAN_FILTERS];
extern CanId motor_can_filter_list[NUM_MOTORCAN_FILTERS];

/**
 * The lookup table containing the entries for all of our CAN messages. Located
 * in kCanLut.c
 */
extern const CanLut kCanLut[kMaxCanId];
#endif