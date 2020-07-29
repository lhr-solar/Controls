import csv
import os
import fcntl


file = "BSP/Simulator/Hardware/Data/Contactors.csv"

#Reads file and returns 1 or 0, 1 is on, 0 is off
def Read_Contactor():
    returnNothing = ["", ""]
    if os.path.isfile(file) == True:    #only execute the following code if the .csv file has been created
        os.makedirs(os.path.dirname(file), exist_ok=True)
        with open(file, 'r') as csvfile:
            fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)    # Lock file before reading
            csvreader = csv.reader(csvfile) 
            Contactor_State = next(csvreader)
            fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)    # Unlock file after reading
            return Contactor_State
    else:
        return returnNothing #csv file hasn't been made, there is no data to read