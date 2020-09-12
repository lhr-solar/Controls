#ifndef __BSP_H
#define __BSP_H

#include "BSP_ADC.h"
#include "BSP_CAN.h"
#include "BSP_UART.h"
#include "BSP_Switches.h"
#include "BSP_Timer.h"
#include "BSP_Contactors.h"
#include "BSP_Precharge.h"

#include <sys/file.h>
#include <unistd.h>

#ifdef SIMULATOR
#define DATA_PATH(f) "BSP/Simulator/Hardware/Data/" f

#define SWITCHES_CSV "Switches.csv"
#define CONTACTORS_CSV "Contactors.csv"
#define PEDALS_CSV "Pedals.csv"
#define UART_CSV "UART.csv"
#define TIMER_CSV "Timer.csv"
#define CAN_CSV "CAN.csv"
#define PRECHARGE_CSV "Precharge.csv"
#endif

#endif
