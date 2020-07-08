/*
C file that reads the raw ADC data from the CSV file, 
converts it back to a distance pressed (percentage for now), and stores both.
We should be able to request both the raw ADC data,
the distance (percentage for now) and millivolts from this library.
 */

//NOTE: ADC precision is 12 bits (4096), range 3.3V
//I'm assuming 3.3V corresponds to a 100% pressed pedal

#include "BSP_ADC.h"

#define FILE_NAME "BSP/Simulator/Hardware/Data/Pedals.csv"
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
        printf("File not found inside BSP/Simulator/Hardware/Data/");
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
int convert_ADC_to_Voltage(int ADCvalue){
    return ((ADCvalue / 4096) * 3300);
}

/**
 * @brief   Helper function that gets
 *          the number of ADC values 
 *          present in the csv file,
 *          each value has to be separated by a comma
 *          or new line
 * @param   none
 * @return  numberOfADCValues, the number of ADC channels connected to the ADC providing raw ADC data
 */ 
int number_Values(){
    int numberOfADCValues = 0;
    //opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    //counting how many ADC Values the file has
    for(char c = getc(filePointer); c!= EOF; c = getc(filePointer)){
        if(c == '\n' || c == ','){
            numberOfADCValues++;
        }
    }
    //close the file 
    fclose(filePointer);
    return numberOfADCValues;
}

/**
 * @brief   Helper function that reads all of the ADC values 
 *          from a file with raw values provided 
 *          by the ADC and stores all values in local 
 *          array of max capacity 10
 * @param   none
 * @return  rawADCValues, pointer to array that contains the ADC values in the array
 */ 
int* read_ADC_Values() {
    //array that will store the ADC values read by the function, it has a maximum capacity of 10
    static int rawADCValues[MAX_CHANNELS];
    //number of values inside the csv file
    int numberOfValues = number_Values();
    //opening file in read mode
    FILE *filePointer = fopen(FILE_NAME, "r");
    //verifying the number of values in the csv file is less than 10
    if(numberOfValues > MAX_CHANNELS){
        numberOfValues = MAX_CHANNELS;
    }
    //reading the ADC values in the csv file and storing them in the rawADCValues array
    for(int i = 0; i < numberOfValues; i++){
        fscanf(filePointer,"%d,", &(rawADCValues[i]));
    }
    //closing the file
    fclose(filePointer);
    //returning array
    return rawADCValues;
}

/**
 * @brief   Function that returns a specific
 *          raw ADC value of a channel, the index of
 *          the channel is provided as a parameter 
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  rawADCValue
 */ 
int16_t BSP_ADC_Get_Value(pedal_t hardwareDevice){
    //checking if indexOfValue is out of bounds
    if(hardwareDevice > number_Values() - 1){
        hardwareDevice = number_Values() - 1;
    }
    //reading all values and storing them in an int array
    int* rawADCValues = read_ADC_Values();
    int16_t rawADCValue = *(rawADCValues + hardwareDevice);
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
int BSP_ADC_Get_Millivoltage(pedal_t hardwareDevice){
    //getting rawADCValue at the specified index
    int rawADCValue = BSP_ADC_Get_Value(hardwareDevice);
    //converting the rawADCValue to millivolts and returning it
    return convert_ADC_to_Voltage(rawADCValue);
}


