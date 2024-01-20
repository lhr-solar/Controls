/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Display.h"
#include "bsp.h"   // for writing to UART
#include "Tasks.h" // for os and fault error codes
#include "fff.h"

#define DISP_OUT UART_3
#define MAX_MSG_LEN 32
#define MAX_ARG_LEN 16
// Assignment commands have only 1 arg, an operator, and an attribute
#define isAssignCmd(cmd) (cmd.compOrCmd != NULL && cmd.op != NULL && cmd.attr != NULL && cmd.numArgs == 1)
// Operational commands have no attribute and no operator, just a command and >= 0 arguments
#define isOpCmd(cmd) (cmd.op == NULL && cmd.attr == NULL)

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Send, DisplayCmd_t);

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Init);

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Reset);

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Error, error_code_t);

DEFINE_FAKE_VALUE_FUNC(DisplayError_t, Display_Evac, uint8_t, uint32_t);