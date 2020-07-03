#include "BSP_ADC.h"

/*
C file that reads the raw ADC data from the CSV file, 
converts it back to a distance pressed (percentage for now), and stores both.
We should be able to request both the raw ADC data,
the distance (percentage for now) and millivolts from this library.
 */

//NOTE: ADC precision is 12 bits (4096), range 3.3V
//I'm assuming 3.3V corresponds to a 100% pressed pedal

#define FILE_NAME "BSP/Simulator/Hardware/Data/Pedals.csv"
#define MAX_CHANNELS 10

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
 *          percentage of the parameter when    
 *          compared to the 4096 possible range 
 *          of the ADC
 * @param   raw ADC value
 * @return  percentage of ADC value 
 */ 
float convert_ADC_to_Percent(int ADCvalue) {
    return (((float)ADCvalue) / 4096) * 100;
}

/**
 * @brief   Helper function which returns a 
 *          milivoltage based on the raw ADC value
 *          compared to the 3.3 volt range of the 
 *          pedals' potentiometers
 * @param   raw ADC value
 * @return  milivolts 
 */ 
float convert_ADC_to_Voltage(int ADCvalue){
    return (((float)ADCvalue / 4096) * 3300);
}

/**
 * @brief   Function that gets
 *          the number of ADC values 
 *          present in the csv file,
 *          each value has to be separated by a comma
 *          or new line
 * @param   none
 * @return  numberOfADCValues, the number of ADC channels connected to the ADC providing raw ADC data
 */ 
int BSP_ADC_Number_Values(){
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
 * @brief   Function that reads all of the ADC values 
 *          from a file with raw values provided 
 *          by the ADC and stores all values in local 
 *          array of max capacity 10
 * @param   none
 * @return  rawADCValues, pointer to array that contains the ADC values in the array
 */ 
int* BSP_ADC_Read_Values() {
    //array that will store the ADC values read by the function, it has a maximum capacity of 10
    static int rawADCValues[MAX_CHANNELS];
    //number of values inside the csv file
    int numberOfValues = BSP_ADC_Number_Values();
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
int BSP_ADC_Get_Value(int indexOfValue){
    //checking if indexOfValue is out of bounds
    if(indexOfValue > BSP_ADC_Number_Values() - 1){
        indexOfValue = BSP_ADC_Number_Values() - 1;
    }
    //reading all values and storing them in an int array
    int* rawADCValues = BSP_ADC_Read_Values();
    //returning the specified value by indexOfValue
    return *(rawADCValues + indexOfValue);
}

/**
 * @brief   Function that returns a specific
 *          raw ADC value in millivolts of a channel, the index of
 *          the channel is provided as a parameter 
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in millivolts
 */ 
float BSP_ADC_Get_Millivoltage(int indexOfValue){
    //getting rawADCValue at the specified index
    int rawADCValue = BSP_ADC_Get_Value(indexOfValue);
    //converting the rawADCValue to millivolts and returning it
    return convert_ADC_to_Voltage(rawADCValue);
}

/**
 * @brief   Function that returns a specific
 *          raw ADC value in percentage of a channel, the index of
 *          the channel is provided as a parameter 
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in percentage
 */ 
float BSP_ADC_Get_Percentage(int indexOfValue){
    //getting rawADCValue at the specified index
    int rawADCValue = BSP_ADC_Get_Value(indexOfValue);
    //converting the rawADCValue to millivolts and returning it
    return convert_ADC_to_Percent(rawADCValue);
}

//** FOLLOWING FUNCTIONS ARE SPECIFIC FOR ACCELERATOR AND BRAKE ADC INFO **///

/**
 * @brief   Getter function that reads the Accelerator ADC,
 *          which we know it's at index 0 of the csv file 
 *          value, converts it to a percentage,  
 *          and returns it
 * @param   None
 * @return  Percentage of Accelerator ADC
 */ 
float BSP_ADC_Accel_GetPercentage() {
     //reading raw ADC values from csv file
    int rawADCValue = BSP_ADC_Get_Value(Accelerator);
    //convert ADC value to a percentage using a helper function and return value
    return convert_ADC_to_Percent(rawADCValue);
}

/**
 * @brief   Getter function that reads the Brake ADC,
 *          which we know it's at index 1 of the csv file 
 *          value, converts it to a percentage,  
 *          and returns it
 * @param   None
 * @return  Percentage of Brake ADC
 */ 
float BSP_ADC_Brake_GetPercentage() {
    //reading raw ADC values from csv file
    int rawADCValue = BSP_ADC_Get_Value(Brake);
    //convert ADC value to a percentage using a helper function and return value
    return convert_ADC_to_Percent(rawADCValue);
}

/**
 * @brief   Getter function that reads
 *          the Accelerator ADC (at index 0) value, 
 *          converts it to millivolts 
 *          and returns it
 * @param   None
 * @return  Millivolts of Accelerator ADC
 */ 
float BSP_ADC_Accel_GetMillivoltage(){
    //reading raw ADC values from csv file
    int rawADCValue = BSP_ADC_Get_Value(Accelerator);
    //convert ADC value to milivolts using a helper function and return value
    return convert_ADC_to_Voltage(rawADCValue);
}

/**
 * @brief   Getter function that reads
 *          the Brake ADC (at index 1) value, 
 *          converts it to millivolts 
 *          and returns it
 * @param   None
 * @return  Millivolts of Brake ADC
 */ 
float BSP_ADC_Brake_GetMillivoltage(){
    //reading raw ADC values from csv file
    int rawADCValue = BSP_ADC_Get_Value(Brake);
    //convert ADC value to milivolts using a helper function and return value
    return convert_ADC_to_Voltage(rawADCValue);
}

