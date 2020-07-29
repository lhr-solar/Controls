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
            try:    #the following code is to prevent simulator from crashing if the csv is blank
                csvreader = csv.reader(csvfile)
                Contactor_State = next(csvreader)
            except StopIteration as stopIt:
                Contactor_State = returnNothing
            fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)    # Unlock file after reading
            return Contactor_State
    else:
        os.makedirs(os.path.dirname(file), exist_ok=True)
        with open(file, 'w') as csvfile:
            fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)    
            fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)   
        return returnNothing #csv file has just been made, there is no data to read