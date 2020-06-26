#ifndef __BSP_H
#define __BSP_H

#include "BSP_ADC.h"
#include "BSP_CAN.h"
#include "BSP_SPI.h"
#include "BSP_Switches.h"
#include "BSP_Timer.h"

#include <sys/file.h>

#ifdef SIMULATOR
#define DATA_PATH(f) "BSP/Simulator/Hardware/Data/" f

#define SWITCHES_CSV "Switches.csv"
#endif

#endif
