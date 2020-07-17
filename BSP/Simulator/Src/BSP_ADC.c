/**
 * C file that reads the raw ADC data from the CSV file, 
 * converts it back to a distance pressed (percentage for now), and stores both.
 * We should be able to request both the raw ADC data,
 * the distance (percentage for now) and millivolts from this library.
 */

// NOTE: ADC precision is 12 bits (4096), range 3.3V
// I'm assuming 3.3V corresponds to a 100% pressed pedal

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
}

/**
 * @brief   Helper function which returns a 
 *          millivoltage based on the raw ADC value
 *          compared to the 3.3 volt range of the 
 *          pedals' potentiometers
 * @param   rawADCvalue ADC data value before conversion
 * @return  millivolts 
 */ 
static int16_t convert_ADC_to_Voltage(int16_t rawADCValue){
    return ((int32_t)rawADCValue)*ADC_RANGE_MILLIVOLTS >> ADC_PRECISION_BITS;
}

/**
 * @brief   Helper function that reads all of the ADC values 
 *          from a file with raw values provided 
 *          by the ADC and stores all values in local 
 *          array of max capacity 10
 * @param   rawADCValues pointer to array that contains the ADC values in the array
 * @return  None
 */ 
static void read_ADC_Values(int16_t *rawADCValues) {
    // Opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    for(int i = 0; i < MAX_CHANNELS; i++){
        // If the file doesn't contain any more values, stop early
        if (fscanf(filePointer,"%hd,", &(rawADCValues[i])) <= 0) break;
    }
    // Closing the file
    fclose(filePointer);
}

/**
 * @brief   Reads the raw ADC value of the specified device
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  Raw ADC value without conversion
 */ 
int16_t BSP_ADC_Get_Value(pedal_t hardwareDevice){
    // Array we'll use to get all the values from csv file
    int16_t rawADCValues[MAX_CHANNELS];
    // Reading all values and storing them in an int array
    read_ADC_Values(rawADCValues);
    // Checking if hardwareDevice is out of bounds
    if(hardwareDevice >  NUMBER_OF_PEDALS - 1 || hardwareDevice < 0){
        //throwing an error
        fprintf(stderr, "Argument passed doesn't exit in ADC file\n");
        exit(EXIT_SUCCESS);
    }
    int16_t rawADCValue = rawADCValues[hardwareDevice];
    // Returning the specified value by indexOfValue
    return rawADCValue;
}

/**
 * @brief   Reads raw ADC data for the specified device and converts
 *          that value into millivolts
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  ADC value in millivolts
 */ 
int16_t BSP_ADC_Get_Millivoltage(pedal_t hardwareDevice){
    // Getting rawADCValue at the specified index
    int16_t rawADCValue = BSP_ADC_Get_Value(hardwareDevice);
    // Converting the rawADCValue to millivolts and returning it
    return convert_ADC_to_Voltage(rawADCValue);
}
