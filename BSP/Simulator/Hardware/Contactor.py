# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

file = "BSP/Simulator/Hardware/Data/Contactors.csv"


def read():
    """Reads contactor file and returns 1 (on) or 0 (off)
    Returns:
        list: [motor_state, array_state]
    """
    os.makedirs(os.path.dirname(file), exist_ok=True)
    if not os.path.exists(file):
        with open(file, 'w'):
            pass

    with open(file, 'r') as csvfile:
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_EX)    # Lock file before reading
        try:    #the following code is to prevent simulator from crashing if the csv is blank
            csvreader = csv.reader(csvfile)
            contactor_states = next(csvreader)
        except StopIteration as stopIt:
            contactor_states = ['', '']
        fcntl.flock(csvfile.fileno(), fcntl.LOCK_UN)    # Unlock file after reading
        return contactor_states 
