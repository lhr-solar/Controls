/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "SendTritium.h"

DEFINE_FAKE_VOID_FUNC(Task_SendTritium, void*);
DEFINE_FAKE_VALUE_FUNC(bool, GetCruiseEnable);
DEFINE_FAKE_VALUE_FUNC(bool, GetCruiseSet);
DEFINE_FAKE_VALUE_FUNC(bool, GetOnePedalEnable);
DEFINE_FAKE_VALUE_FUNC(bool, GetRegenEnable);
DEFINE_FAKE_VALUE_FUNC(uint8_t, GetBrakePedalPercent);
DEFINE_FAKE_VALUE_FUNC(uint8_t, GetAccelPedalPercent);
DEFINE_FAKE_VALUE_FUNC(Gear, GetGear);
DEFINE_FAKE_VALUE_FUNC(TritiumState, GetState);
DEFINE_FAKE_VALUE_FUNC(float, GetVelocityObserved);
DEFINE_FAKE_VALUE_FUNC(float, GetCruiseVelSetpoint);
DEFINE_FAKE_VALUE_FUNC(float, GetCurrentSetpoint);
DEFINE_FAKE_VALUE_FUNC(float, GetVelocitySetpoint);

#ifdef SENDTRITIUM_EXPOSE_VARS
DEFINE_FAKE_VOID_FUNC(SetCruiseEnable, bool);
DEFINE_FAKE_VOID_FUNC(SetCruiseSet, bool);
DEFINE_FAKE_VOID_FUNC(SetOnePedalEnable, bool);
DEFINE_FAKE_VOID_FUNC(SetRegenEnable, bool);
DEFINE_FAKE_VOID_FUNC(SetBrakePedalPercent, uint8_t);
DEFINE_FAKE_VOID_FUNC(SetAccelPedalPercent, uint8_t);
DEFINE_FAKE_VOID_FUNC(SetGear, Gear);
DEFINE_FAKE_VOID_FUNC(SetState, TritiumState);
DEFINE_FAKE_VOID_FUNC(SetVelocityObserved, float);
DEFINE_FAKE_VOID_FUNC(SetCruiseVelSetpoint, float);
DEFINE_FAKE_VOID_FUNC(SetCurrentSetpoint, float);
DEFINE_FAKE_VOID_FUNC(SetVelocitySetpoint, float);
#endif