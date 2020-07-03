#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "config.h"

enum ADCHardware {Accelerator, Brake};

/**
 * @brief   Initialize the ADC module
 * @param   None
 * @return  None
 */ 
void BSP_ADC_Init(void);

/**
 * @brief   Provides the number of active ADC channels 
 * @param   None
 * @return  numberOfADCValues, the number of ADC channels connected to the ADC providing raw ADC data
 */ 
int BSP_ADC_Number_Values();

/**
 * @brief   Provides pointer to array with the ADC values of csv file
 * @param   None
 * @return  rawADCValues, pointer to array that contains the ADC values in the array
 */ 
int* BSP_ADC_Read_Values();

/**
 * @brief   Provides the ADC value of the channel at the specified index
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  rawADCValue
 */ 
int BSP_ADC_Get_Value(int indexOfValue);

/**
 * @brief   Provides the ADC value in millivolts of the channel at the specified index
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in millivolts
 */ 
float BSP_ADC_Get_Millivoltage(int indexOfValue);

/**
 * @brief   Provides the ADC value in percentage of the channel at the specified index
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in percentage
 */ 
float BSP_ADC_Get_Percentage(int indexOfValue);

/**
 * @brief   Getter function that returns the Accelerator ADC value in percentage
 * @param   None
 * @return  Percentage of Accelerator ADC
 */ 
float BSP_ADC_Accel_GetPercentage();

/**
 * @brief   Getter function that returns the Brake ADC value in percentage
 * @param   None
 * @return  Percentage of Brake ADC
 */ 
float BSP_ADC_Brake_GetPercentage();

/**
 * @brief   Getter function that returns the Accelerator ADC value in mV
 * @param   None
 * @return  Millivolts of Accelerator ADC
 */ 
float BSP_ADC_Accel_GetMillivoltage();

/**
 * @brief   Getter function that returns the Brake ADC value in mV
 * @param   None
 * @return  Millivolts of Brake ADC
 */ 
float BSP_ADC_Brake_GetMillivoltage();

#endif