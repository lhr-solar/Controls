/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "SendTritium.h"
#ifndef TEST_SENDTRITIUM

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Task_SendTritium, void*);

// Getter functions for local variables in SendTritium.c
DECLARE_FAKE_VALUE_FUNC(bool, GetCruiseEnable);
DECLARE_FAKE_VALUE_FUNC(bool, GetCruiseSet);
DECLARE_FAKE_VALUE_FUNC(bool, GetOnePedalEnable);
DECLARE_FAKE_VALUE_FUNC(bool, GetRegenEnable);
DECLARE_FAKE_VALUE_FUNC(uint8_t, GetBrakePedalPercent);
DECLARE_FAKE_VALUE_FUNC(uint8_t, GetAccelPedalPercent);
DECLARE_FAKE_VALUE_FUNC(Gear, GetGear);
DECLARE_FAKE_VALUE_FUNC(TritiumState, GetState);
DECLARE_FAKE_VALUE_FUNC(float, GetVelocityObserved);
DECLARE_FAKE_VALUE_FUNC(float, GetCruiseVelSetpoint);
DECLARE_FAKE_VALUE_FUNC(float, GetCurrentSetpoint);
DECLARE_FAKE_VALUE_FUNC(float, GetVelocitySetpoint);

// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
DECLARE_FAKE_VOID_FUNC(SetCruiseEnable, bool);
DECLARE_FAKE_VOID_FUNC(SetCruiseSet, bool);
DECLARE_FAKE_VOID_FUNC(SetOnePedalEnable, bool);
DECLARE_FAKE_VOID_FUNC(SetRegenEnable, bool);
DECLARE_FAKE_VOID_FUNC(SetBrakePedalPercent, uint8_t);
DECLARE_FAKE_VOID_FUNC(SetAccelPedalPercent, uint8_t);
DECLARE_FAKE_VOID_FUNC(SetGear, Gear);
DECLARE_FAKE_VOID_FUNC(SetState, TritiumState);
DECLARE_FAKE_VOID_FUNC(SetVelocityObserved, float);
DECLARE_FAKE_VOID_FUNC(SetCruiseVelSetpoint, float);
DECLARE_FAKE_VOID_FUNC(SetCurrentSetpoint, float);
DECLARE_FAKE_VOID_FUNC(SetVelocitySetpoint, float);
#endif

#endif