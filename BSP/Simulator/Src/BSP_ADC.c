/*
C file that reads the raw ADC data from the CSV file, 
converts it back to a distance pressed (percentage for now), and stores both.
We should be able to request both the raw ADC data,
the distance (percentage for now) and millivolts from this library.
 */

//NOTE: ADC precision is 12 bits (4096), range 3.3V
//I'm assuming 3.3V corresponds to a 100% pressed pedal

#include "BSP_ADC.h"

#define FILE_NAME DATA_PATH(PEDALS_CSV)

/**
 * @brief   Confirms that the CSV file
 *          has been created and prints and 
 *          error message if not
 * @param   None
 * @return  None
 */ 
void BSP_ADC_Init(void) {
    // Checking if file exists
   if(access(FILE_NAME, F_OK) != 0){
        // File doesn't exist if true
        perror(PEDALS_CSV);
        exit(EXIT_FAILURE);
    }
    // There really isn't much we have to do in the simulator
}

/**
 * @brief   Helper function which returns a 
 *          milivoltage based on the raw ADC value
 *          compared to the 3.3 volt range of the 
 *          pedals' potentiometers
 * @param   raw ADC value
 * @return  milivolts 
 */ 
int16_t convert_ADC_to_Voltage(int16_t ADCvalue){
    return (((float)ADCvalue / 4096) * 3300);
}

/**
 * @brief   Helper function that reads all of the ADC values 
 *          from a file with raw values provided 
 *          by the ADC and stores all values in local 
 *          array of max capacity 10
 * @param   none
 * @return  rawADCValues, pointer to array that contains the ADC values in the array
 */ 
void read_ADC_Values(int16_t *rawADCValues) {
    //opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    //TODO: Correct the following function, read all values till the end of file
    for(int i = 0; i < 2; i++){
        fscanf(filePointer,"%d,", &(rawADCValues[i]));
    }
    //closing the file
    fclose(filePointer);
}

/**
 * @brief   Function that returns a specific
 *          raw ADC value of a channel, the index of
 *          the channel is provided as a parameter 
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  rawADCValue
 */ 
int16_t BSP_ADC_Get_Value(pedal_t hardwareDevice){
    //array we'll use to get all the values from csv file
    int16_t rawADCValues[MAX_CHANNELS];
    //reading all values and storing them in an int array
    read_ADC_Values(rawADCValues);
    //checking if hardwareDevice is out of bounds
    if(hardwareDevice >  NUMBEROFPEDALS - 1 || hardwareDevice < 0){
        //throwing an error
        fprintf(stderr, "Argument passed doesn't exit in ADC file\n");
        exit(EXIT_SUCCESS);
    }
    int16_t rawADCValue = rawADCValues[hardwareDevice];
    //returning the specified value by indexOfValue
    return rawADCValue;
}

/**
 * @brief   Function that returns a specific
 *          raw ADC value in millivolts of a channel, the index of
 *          the channel is provided as a parameter 
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in millivolts
 */ 
int16_t BSP_ADC_Get_Millivoltage(pedal_t hardwareDevice){
    //getting rawADCValue at the specified index
    int16_t rawADCValue = BSP_ADC_Get_Value(hardwareDevice);
    //converting the rawADCValue to millivolts and returning it
    return convert_ADC_to_Voltage(rawADCValue);
}


