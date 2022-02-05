#ifndef __FAULT_STATE_H
#define __FAULT_STATE_H

#include "os.h"
#include "Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "config.h"
#include "MotorConnection.h"
#include "ArrayConnection.h"

void EnterFaultState(void *p_arg);

#endif