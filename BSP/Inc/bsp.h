/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __BSP_H
#define __BSP_H

#include "BSP_ADC.h"
#include "BSP_CAN.h"
#include "BSP_UART.h"
#include "BSP_SPI.h"
#include "BSP_GPIO.h"

#include <sys/file.h>
#include <unistd.h>

#ifdef SIMULATOR
#define DATA_PATH(f) "BSP/Simulator/Hardware/Data/" f

#define SWITCHES_CSV "Switches.csv"
#define CONTACTORS_CSV "Contactors.csv"
#define PEDALS_CSV "Pedals.csv"
#define UART_CSV "UART.csv"
#define CAN_CSV "CAN.csv"
#define SPI_CSV "SPI.csv"
#define LIGHTS_CSV "Lights.csv"
#define PRECHARGE_CSV "PreCharge.csv"
#define GPIO_CSV "GPIO.csv"
#endif

#endif
