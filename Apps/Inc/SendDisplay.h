#ifndef __SEND_DISPLAY_H
#define __SEND_DISPLAY_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

#include "Display.h"
#include "Contactors.h"

/**
 * @brief Selects visible page on the display
 * @param page which page to select
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetPage(Page_t page);

/**
 * @brief Sets the state of charge value on the display
 * @param percent charge as a percent
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetSOC(uint8_t percent);

/**
 * @brief Sets the supplemental battery pack voltage value on the display
 * @param mv supplemental battery pack voltage in millivolts
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetSBPV(uint32_t mv);

/**
 * @brief Sets the velocity of the vehicle on the display
 * @param mphTenths velocity of the vehicle in tenths of mph
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetVelocity(uint32_t mphTenths);

/**
 * @brief Sets the accelerator slider value on the display
 * @param percent pressure on accelerator in percent
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetAccel(uint8_t percent);

/**
 * @brief Sets the array indicator state on the display
 * @param state on or off
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetArray(bool state);

/**
 * @brief Sets the motor indicator state on the display
 * @param state on or off
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetMotor(bool state);

/**
 * @brief Sets the gear selection state on the display
 * @param gear DISABLED=N, ENABLED=F, ACTIVE=R
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetGear(TriState_t gear);

/**
 * @brief Sets the regenerative braking indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetRegenState(TriState_t state);

/**
 * @brief Sets the cruise control indicator state on the display
 * @param state DISABLED, ENABLED, or ACTIVE
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetCruiseState(TriState_t state);

/**
 * @brief Sets the left blinker state on the display
 * @param state blinking or not blinking
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetLeftBlink(bool state);

/**
 * @brief Sets the right blinker state on the display
 * @param state blinking or not blinking
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetRightBlink(bool state);

/**
 * @brief Sets the headlight state on the display
 * @param state on or off
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_SetHeadlight(bool state);

#endif