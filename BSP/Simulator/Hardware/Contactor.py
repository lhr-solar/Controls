import csv
import os
import fcntl


file = "BSP/Simulator/Hardware/Data/Contactors.csv"

def read():
    """Reads contactor file and returns 1 or 0, 1 is on, 0 is off
    Returns:
        list: [motor_state, array_state]
    """
    returnNothing = ["", ""]
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        open(file, 'w')

    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)    # Lock file before reading
        try:    #the following code is to prevent simulator from crashing if the csv is blank
            csvreader = csv.reader(csvfile)
            Contactor_State = next(csvreader)
        except StopIteration as stopIt:
            Contactor_State = returnNothing
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)    # Unlock file after reading
        return Contactor_State 
        