#We open and write to CSV file to simulate CAN read/write. This will put the info in it we need for CAN stuff
#Once this CSV file has the info needed (the values in the flags representing different errors)
#we can run the Test_MotorConErrorMessages.c file and see if the outputs are what we expected, it will use info from the CSV file

#lines that account for info needed in CSV file
#uint32_t length = BSP_CAN_Read(CAN_2, &id, data);

#@brief   Reads the message on the specified CAN line
#@param   id pointer to integer to store the 
#          message ID that was read
#@param   data pointer to integer array to store
#          the message in bytes
#@return  number of bytes read (0 if unsuccessful)

#uint8_t BSP_CAN_Read(CAN_t bus, uint32_t* id, uint8_t* data) {
#    FILE* fp = fopen(FILE_NAME, "r");
#    if (!fp) {
#        printf("CAN not available\n\r");
#        return 0;}

#write to CAN_2 so this info is here for data:  data[8] = {0x00, 0x00, 0x00, 0x00,      0x01, 0x08, 0x00, 0x44};
#write to CAN_2 so this info is here for data:  data[8] = {0x00, 0x00, 0x00, 0x00,      0x00, 0x08, 0x00, 0x44};
#write to CAN_2 so this info is here for data:  data[8] = {0x00, 0x00, 0x00, 0x00,      0x00, 0x08, 0x00, 0x40};
#write to CAN_2 so this info is here for data:  data[8] = {0x00, 0x00, 0x00, 0x00,      0x00, 0x00, 0x00, 0x40};
#write to CAN_2 so this info is here for data:  data[8] = {0x00, 0x00, 0x00, 0x00,      0x00, 0x00, 0x00, 0x00};

#write to csv file by hand if BSP functions in c don't read it correctly, but otherwise, BSP functions should read csv files normally
#will the call to BSP_CAN_Read from the Read_MotorController function in MotorController.c read the csv data right? We'll see

import os.path
import csv
from MotorController import read, write
#import functions that read and write to CAN, is this needed?

CANFile =open('BSP/Simulator/Hardware/Data/CAN.csv', 'w')  #open the file
#CANFile="MotorCon.csv"

# create the csv writer
CANWriter = csv.writer(CANFile)

#This section should create the data that BSP_CAN_Read will use for motor messages
# write the initial data to CSV file
#CAN_3 reads line 2, we use this CAN line
#can either rewrite lines in one csv file, or make multiple csv files. Will figure out rewrite later, how to get CAN to read??

write(1, 17301572) #all flags
write(1, 524356) #overspeed off
write(1, 524352)
write(1, 64)
write(1, 0)

# close the file
CANFile.close()




