#include "bsp.h"
#include "Display.h"
#include <math.h>
#include <string.h>

#define NEXTION_INSTRUCTION_SUCCESSFUL 0x01
#define DISP_OUT UART_3
// The conversion factor between meters per second to deci-miles per hour (3.6 / 1.609 * 10)
#define MPS_TO_dMPH 22.374f

#define CHECK(expr)      \
    if ((expr) == ERROR) \
    return ERROR
static const char *DELIMITER = ".";
static const char *ASSIGNMENT = "=";
static const char *TERMINATOR = "\xff\xff\xff";
static const char *NO_ERROR = "No Error";

// Color defintions for display
static const uint16_t NEXTION_GREEN = 2016;
static const uint16_t NEXTION_RED = 63488;
//static const uint16_t NEXTION_LIGHT_GREY = 42260;
// static const uint16_t NEXTION_DARK_GREY = 23275;
// static const uint16_t NEXTION_BURNT_ORANGE = 51872;

static inline int IsNextionFailure(uint32_t val)
{
    return (val >> 24) != 1;
}

// List of names of possible strings to build a command out of
enum CommandString_t
{
    VELOCITY,
    CRUISE_ENABLE,
    CRUISE_SET,
    REGEN_ENABLE,
    VALUE,
    TEXT,
    PCO,
    VIS,
    SYSTEM,
    PAGE,
    ERROR0,
    ERROR1,
    ERROR2,
    ERROR3,
    ERROR4,
    ERROR5,
    SUPPL_VOLT,
    CHARGE_STATE
};

// The command strings themselves
static char *CommandStrings[] = {
    "x0",
    "t1",
    "t2",
    "t3",
    "val",
    "txt",
    "pco",
    "vis",
    "",
    "page",
    "t4",
    "t5",
    "t6",
    "t7",
    "t8",
    "t9",
    "x1",
    "x2"};

/**
 * Sends a string of the form "obj_index.attr_index=" or "attr_index=" over UART
 * Do not call on its own, should only be called by the updateValue subroutines
 */
static void sendStartOfAssignment(enum CommandString_t obj_index, enum CommandString_t attr_index)
{
    char *obj = CommandStrings[obj_index];
    char *attr = CommandStrings[attr_index];
    int len = strlen(obj);

    if (len != 0)
    { // If not global
        BSP_UART_Write(DISP_OUT, obj, len);
        BSP_UART_Write(DISP_OUT, (char *)DELIMITER, strlen(DELIMITER));
    }

    BSP_UART_Write(DISP_OUT, attr, strlen(attr)); // Send the attribute
    BSP_UART_Write(DISP_OUT, (char *)ASSIGNMENT, 1);
}

/**
 * Sends out a string of the form "obj.attr=msg" or "attr=msg"
 * Use to update string fields of display objects
 */
static ErrorStatus updateStringValue(enum CommandString_t obj_index, enum CommandString_t attr_index, char *msg)
{
    sendStartOfAssignment(obj_index, attr_index);

    BSP_UART_Write(DISP_OUT, msg, strlen(msg));
    BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

    // Get a response from the display
    char buf[8];
    BSP_UART_Read(DISP_OUT, buf);
    return (IsNextionFailure(buf[0])) ? ERROR : SUCCESS;
}

/**
 * Sends out a string of the form "obj.attr=val" or "attr=val"
 * Use to update integer fields of display objects
 */
static ErrorStatus updateIntValue(enum CommandString_t obj_index, enum CommandString_t attr_index, int32_t val)
{
    sendStartOfAssignment(obj_index, attr_index);

    char number[12]; // To store converted int
    sprintf(number, "%ld", val);

    BSP_UART_Write(DISP_OUT, number, strlen(number));
    BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

    // Get a response from the display
    char buf[8];
    BSP_UART_Read(DISP_OUT, buf);
    return (IsNextionFailure(buf[0])) ? ERROR : SUCCESS;
}

/*
 * Sets a component's visiblity
 */
static ErrorStatus setComponentVisibility(enum CommandString_t comp, bool vis)
{
    char out[24];
    sprintf(out, "%s %s,%d%s", CommandStrings[VIS], CommandStrings[comp], vis ? 1 : 0, TERMINATOR);

    BSP_UART_Write(DISP_OUT, out, strlen(out));

    char buf[8];
    BSP_UART_Read(DISP_OUT, buf);
    return (IsNextionFailure(buf[0]) ? ERROR : SUCCESS);
}

/*
 * Initialize the Nextion display
 */
void Display_Init()
{
    char ret[8];
    for (int i = 0; i < 8; i++)
        ret[i] = 0;
    volatile char *x = ret;
    BSP_UART_Init(DISP_OUT);
    // The display sends 0x88 when ready, but that might be
    // before we initialize our UART
    BSP_UART_Read(DISP_OUT, (char *)x);
    if (ret[0] == 0x88)
    {
        volatile int a = 0;
        while (1)
            a++;
    }
}

/**
 * Set the displayed velocity to vel
 */
ErrorStatus Display_SetVelocity(float vel)
{
    int32_t vel_fix = (uint32_t)floorf(vel * MPS_TO_dMPH);
    return updateIntValue(VELOCITY, VALUE, vel_fix);
}

/**
 * @brief Update the charge state on the display.
 * @param chargeState
 * @return void
 */
ErrorStatus Display_SetChargeState(uint32_t chargeState)
{
    int32_t format = (int32_t)(chargeState / 100000); // charge state comes in 6 digit precision, we only care about the integer and one decimal places
    return updateIntValue(CHARGE_STATE, VALUE, format);
}

/**
 * @brief Updates the display with whether regenerative braking / charging is allowed or not
 * @param ChargeEnabled a state value indicating whether or not charging is enabled
 */
ErrorStatus Display_SetRegenEnabled(State ChargeEnabled)
{
    if (ChargeEnabled == ON)
    {
        return setComponentVisibility(REGEN_ENABLE, true);
    }
    else
    {
        return setComponentVisibility(REGEN_ENABLE, false);
    }
}

/**
 * Set the displayed supplemental battery pack voltage.
 * Units of millivolts
 */
ErrorStatus Display_SetSBPV(uint16_t mv)
{
    int32_t sbpv = mv / 100; // One tenth of a volt precision
    return updateIntValue(SUPPL_VOLT, VALUE, sbpv);
}

/**
 * Update the cruise enable light on the display based on the given state
 */
ErrorStatus Display_CruiseEnable(State on)
{
    if (on == ON)
    {
        return setComponentVisibility(CRUISE_ENABLE, true);
    }
    else
    {
        return setComponentVisibility(CRUISE_ENABLE, false);
    }
}


/**
 * Set ERROR<idx> to err
 * If err is an empty string, then the error will be cleared
 */
ErrorStatus Display_SetError(int idx, char *err)
{
    if (idx < 0 || idx > 5)
        return ERROR; // Index out of bounds
    if (strlen(err) == 0)
    {
        CHECK(setComponentVisibility(ERROR0 + idx, false)); // Hide text if no error
    }
    else
    {
        CHECK(updateStringValue(ERROR0 + idx, TEXT, err)); // Set error string
        CHECK(updateIntValue(ERROR0 + idx, PCO, NEXTION_RED));
        CHECK(setComponentVisibility(ERROR0 + idx, true));
    }
    return SUCCESS;
}

/**
 * Set the first error slot to say "No Error" and set its color to green
 * User must clear the remaining slots manually using Display_SetError
 */
ErrorStatus Display_NoErrors(void)
{
    CHECK(updateStringValue(ERROR0, TEXT, (char *)NO_ERROR));
    CHECK(updateIntValue(ERROR0, PCO, NEXTION_GREEN));
    CHECK(setComponentVisibility(ERROR0, true));
    return SUCCESS;
}

/**
 * Set the display to the main view
 */
ErrorStatus Display_SetMainView(void)
{
    //return updateIntValue(SYSTEM, PAGE, 1);
    char *page = "page1\xff\xff\xff";    // hopefully this works, changed name of page from page 1 to main
    BSP_UART_Write(DISP_OUT, page, strlen(page));
    return SUCCESS;
}