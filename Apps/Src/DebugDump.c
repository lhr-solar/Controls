#include <errno.h>

#include "CanBus.h"
#include "Contactors.h"
#include "Minions.h"
#include "Pedals.h"
#include "SendTritium.h"
#include "Tasks.h"
#include "bsp.h"
#include "common.h"
#include "os.h"

static const char *minionpin_string[] = {FOREACH_PIN(GENERATE_STRING)};
static const char *contactor_string[] = {FOREACH_CONTACTOR(GENERATE_STRING)};
static const char *gear_string[] = {FOREACH_GEAR(GENERATE_STRING)};

// Need to keep this in sync with Task.h
/*----------------------------------------------*/
#define FAULT_BITMAP_NUM 6

/*----------------------------------------------*/

void TaskDebugDump(void *p_arg) {
    OS_ERR err = 0;

    while (1) {
        // Get pedal information
        int8_t accel_pedal = PedalsRead(kAccelerator);
        printf("kAccelerator: %d\n\r", accel_pedal);

        int8_t brake_pedal = PedalsRead(kBrake);
        printf("kBrake: %d\n\r", brake_pedal);

        // Get minion information
        for (Pin pin = 0; pin < kNumPins; pin++) {
            bool pin_state = MinionsRead(pin);
            // Ignition pins are negative logic, special-case them
            printf("%s: %s\n\r", minionpin_string[pin],
                   pin_state ^ (pin == kIgn1 || pin == kIgn2) ? "on" : "off");
        }

        // Get contactor info
        for (Contactor contactor = 0; contactor < kNumContactors; contactor++) {
            bool contactor_state =
                ContactorsGet(contactor) == ON ? true : false;
            printf("%s: %s\n\r", contactor_string[contactor],
                   contactor_state ? "on" : "off");
        }

        // Send Tritium variables
        printf("Cruise Enable: %s\n\r", GetCruiseEnable() ? "true" : "false");
        printf("Cruise Set: %s\n\r", GetCruiseSet() ? "true" : "false");
        printf("One Pedal Enable: %s\n\r",
               GetOnePedalEnable() ? "true" : "false");
        printf("Regen Enable: %s\n\r", GetRegenEnable() ? "true" : "false");
        printf("Pedal Brake Percent: %d\n\r", GetBrakePedalPercent());
        printf("Pedal Accel Percent: %d\n\r", GetAccelPedalPercent());
        printf("Current Gear: %s\n\r", gear_string[GetGear()]);
        PrintFloat("Current Setpoint: ", GetCurrentSetpoint());

        printf("\n\r");

        // Delay of 5 seconds
        OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err);  // NOLINT
        if (err != OS_ERR_NONE) {
            ASSERT_OS_ERROR(err);
        }

#ifdef MOCKING
        break;
#endif
    }
}