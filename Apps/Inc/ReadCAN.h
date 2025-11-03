#pragma once

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"
#include "os_cfg_app.h"
#include "CANbus.h"
#include "DebugIO.h"
#include "StatusLeds.h"
#include "daybreak_pins.h"

// Semaphore indicates when a new message is availible on carcan
OS_SEM carcan_recv_Sem4;

typedef struct {        
    CANDATA_T data;
    OS_SEM can_recv_Sem4;
}can_recv_entry_t;